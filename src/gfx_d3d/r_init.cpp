#include "r_init.h"
#include <qcommon/mem_track.h>
#include <qcommon/threads.h>
#include <win32/win_local.h>
#include <win32/win_localize.h>
#include "r_dvars.h"
#include "rb_backend.h"
#include "r_state.h"
#include "rb_state.h"
#include "rb_light.h"
#include "r_light.h"
#include "r_staticmodelcache.h"
#include "r_cmds.h"
#include "rb_logfile.h"
#include "r_cinematic.h"
#include "rb_drawprofile.h"
#include "r_image.h"
#include "r_water.h"
#include "r_fog.h"
#include "r_workercmds.h"
#include <database/database.h>
#include "r_scene.h"
#include "r_rendertarget.h"
#include "r_buffers.h"
#include "rb_sky.h"
#include "r_bsp.h"
#include "r_utils.h"
#include "r_draw_method.h"
#include "r_model.h"

#ifdef KISAK_MP
#include <game_mp/g_public_mp.h>
#endif

enum DxCapsResponse : __int32
{                                       // ...
    DX_CAPS_RESPONSE_QUIT = 0x0,  // ...
    DX_CAPS_RESPONSE_WARN = 0x1,  // ...
    DX_CAPS_RESPONSE_INFO = 0x2,  // ...
    DX_CAPS_RESPONSE_FORBID_SM3 = 0x3,  // ...
};

struct DxCapsCheckBits // sizeof=0x14
{                                       // ...
    int offset;                         // ...
    unsigned int setBits;               // ...
    unsigned int clearBits;             // ...
    DxCapsResponse response;            // ...
    const char *msg;                    // ...
};

const DxCapsCheckBits s_capsCheckBits[33] =
{
  {
    12,
    0u,
    536870912u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support dynamic textures"
  },
  { 12, 0u, 131072u, DX_CAPS_RESPONSE_WARN, "doesn't support fullscreen gamma" },
  { 16, 0u, 32u, DX_CAPS_RESPONSE_QUIT, "doesn't support alpha blending" },
  { 16, 0u, 256u, DX_CAPS_RESPONSE_WARN, "doesn't accelerate dynamic textures" },
  {
    20,
    0u,
    2147483648u,
    DX_CAPS_RESPONSE_WARN,
    "doesn't support immediate frame buffer swapping"
  },
  { 20, 0u, 1u, DX_CAPS_RESPONSE_WARN, "doesn't support vertical sync" },
  {
    28,
    0u,
    32768u,
    DX_CAPS_RESPONSE_QUIT,
    "is not at least DirectX 7 compliant"
  },
  {
    28,
    0u,
    66560u,
    DX_CAPS_RESPONSE_WARN,
    "doesn't accelerate transform and lighting"
  },
  { 28, 0u, 524288u, DX_CAPS_RESPONSE_WARN, "doesn't accelerate rasterization" },
  { 32, 0u, 2u, DX_CAPS_RESPONSE_QUIT, "can't disable depth buffer writes" },
  {
    32,
    0u,
    128u,
    DX_CAPS_RESPONSE_QUIT,
    "can't disable individual color channel writes"
  },
  {
    32,
    0u,
    2048u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support frame buffer blending ops beside add"
  },
  {
    32,
    0u,
    131072u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support separate alpha blend, glow will be disabled"
  },
  {
    32,
    0u,
    112u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support all face culling modes"
  },
  {
    36,
    0u,
    33554432u,
    DX_CAPS_RESPONSE_INFO,
    "doesn't support high-quality polygon offset"
  },
  {
    40,
    0u,
    141u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support the required depth comparison modes"
  },
  {
    44,
    0u,
    1023u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support the required frame buffer source blend modes"
  },
  {
    48,
    0u,
    1023u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support the required frame buffer destination blend modes"
  },
  {
    52,
    0u,
    210u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support the required alpha comparison modes"
  },
  { 60, 0u, 4u, DX_CAPS_RESPONSE_QUIT, "doesn't support alpha in textures" },
  { 60, 0u, 2048u, DX_CAPS_RESPONSE_QUIT, "doesn't support cubemap textures" },
  {
    60,
    0u,
    16384u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support mipmapped textures"
  },
  {
    60,
    2u,
    256u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support restricted use of non-power-of-2 textures"
  },
  {
    60,
    0u,
    1u,
    DX_CAPS_RESPONSE_WARN,
    "doesn't support perspective correct texturing"
  },
  { 60, 32u, 0u, DX_CAPS_RESPONSE_QUIT, "doesn't support non-square textures" },
  {
    64,
    0u,
    50529024u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support the required texture filtering modes"
  },
  {
    68,
    0u,
    50332416u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support the required cubemap texture filtering modes"
  },
  { 76, 0u, 4u, DX_CAPS_RESPONSE_QUIT, "doesn't support texture clamping" },
  { 76, 0u, 1u, DX_CAPS_RESPONSE_QUIT, "doesn't support texture wrapping" },
  {
    136,
    0u,
    511u,
    DX_CAPS_RESPONSE_INFO,
    "doesn't support the required stencil operations"
  },
  {
    212,
    0u,
    1u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support vertex stream offsets"
  },
  {
    244,
    0u,
    512u,
    DX_CAPS_RESPONSE_WARN,
    "doesn't support linear filtering when copying and shrinking the frame buffer"
  },
  { 236, 0u, 1u, DX_CAPS_RESPONSE_QUIT, "doesn't support UBYTE4N vertex data" }
}; // idb

struct DxCapsCheckInteger // sizeof=0x14
{                                       // ...
    int offset;                         // ...
    unsigned int min;                   // ...
    unsigned int max;                   // ...
    DxCapsResponse response;            // ...
    const char *msg;                    // ...
};
const DxCapsCheckInteger s_capsCheckInt[10] =
{
  {
    88,
    2048u,
    4294967295u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support large enough 2D textures"
  },
  {
    92,
    2048u,
    4294967295u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support large enough 2D textures"
  },
  {
    96,
    256u,
    4294967295u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support large enough 3D textures"
  },
  {
    148,
    8u,
    4294967295u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support enough texture coordinates for the DirectX 9 code path"
  },
  {
    152,
    8u,
    4294967295u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support enough textures for the DirectX 9 code path"
  },
  { 188, 1u, 4294967295u, DX_CAPS_RESPONSE_QUIT, "is not a DirectX 9 driver" },
  {
    196,
    4294836736u,
    4294901759u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support vertex shader 2.0 or better"
  },
  {
    204,
    4294902272u,
    4294967295u,
    DX_CAPS_RESPONSE_QUIT,
    "doesn't support pixel shader 2.0 or better"
  },
  {
    196,
    4294836992u,
    4294901759u,
    DX_CAPS_RESPONSE_FORBID_SM3,
    "doesn't support vertex shader 3.0 or better"
  },
  {
    204,
    4294902528u,
    4294967295u,
    DX_CAPS_RESPONSE_FORBID_SM3,
    "doesn't support pixel shader 3.0 or better"
  }
}; // idb

//int g_disableRendering   85825680     gfx_d3d:r_init.obj
//struct GfxAssets gfxAssets 85825684     gfx_d3d :r_init.obj
GfxAssets gfxAssets;
//struct DxGlobals dx        85825688     gfx_d3d :r_init.obj
DxGlobals dx;
//struct r_global_permanent_t rgp 85825700     gfx_d3d :r_init.obj
r_global_permanent_t rgp;
//struct vidConfig_t vidConfig 85827880     gfx_d3d :r_init.obj
vidConfig_t vidConfig;
//struct GfxConfiguration gfxCfg 858278b8     gfx_d3d : r_init.obj
//struct GfxGlobals r_glob   858278d0     gfx_d3d : r_init.obj
//struct r_globals_t rg      858278e0     gfx_d3d :r_init.obj
//r_globals_t rg; // LWSS: moved to db_registry for DEDICATED
//struct GfxMetrics gfxMetrics 85827c00     gfx_d3d : r_init.obj
GfxMetrics gfxMetrics;
//int marker_r_init        85827c18     gfx_d3d : r_init.obj
//BOOL g_allocateMinimalResources 85827c1c     gfx_d3d : r_init.obj
bool g_allocateMinimalResources;
GfxConfiguration gfxCfg{ 0 };
GfxGlobals r_glob;

int g_disableRendering;

const dvar_t *r_mode;
const dvar_t *r_displayRefresh;
const dvar_t* r_noborder;

void __cdecl R_SyncGpu(int(__cdecl *WorkCallback)(unsigned __int64))
{
    int useWorkCallback; // [esp+30h] [ebp-4h]

    if (dx.gpuSync)
    {
        PROF_SCOPED("R_SyncGpu");
        useWorkCallback = WorkCallback != 0;
        dx.gpuSyncStart = __rdtsc();
        R_AcquireGpuFenceLock();
        while (!R_GpuFenceTimeout())
        {
            if (useWorkCallback)
            {
                R_ReleaseGpuFenceLock();
                useWorkCallback = (WorkCallback)(dx.gpuSyncEnd);
                R_AcquireGpuFenceLock();
            }
            else
            {
                R_ProcessWorkerCmdsWithTimeout((int(*)())R_GpuFenceTimeout, 0);
            }
        }
        R_ReleaseGpuFenceLock();
    }
}

bool __cdecl R_IsUsingAdaptiveGpuSync()
{
    return dx.gpuSync == 1;
}

void __cdecl TRACK_r_init()
{
    track_static_alloc_internal(&rgp, 8576, "rgp", 18);
    track_static_alloc_internal(&rg, 33552, "rg", 18);
    track_static_alloc_internal(&vidConfig, 48, "vidConfig", 18);
    track_static_alloc_internal(&dx, 11488, "dx", 18);
}

void __cdecl Sys_DirectXFatalError()
{
    HWND ActiveWindow; // eax
    char *v1; // [esp-Ch] [ebp-Ch]
    char *v2; // [esp-8h] [ebp-8h]

    Sys_EnterCriticalSection(CRITSECT_FATAL_ERROR);
    v2 = Win_LocalizeRef("WIN_DIRECTX_INIT_TITLE");
    v1 = Win_LocalizeRef("WIN_DIRECTX_INIT_BODY");
    ActiveWindow = GetActiveWindow();
    MessageBoxA(ActiveWindow, v1, v2, 0x10u);
    ShellExecuteA(0, "open", "Docs\\TechHelp\\Tech Help\\Information\\DirectX.htm", 0, 0, 3);
    exit(-1);
}

const char *__stdcall DXGetErrorDescription9A(int a1)
{
    const char *result; // eax

    if (a1 > -2005532232)
    {
        if (a1 > -2005397200)
        {
            if (a1 <= -2005397136)
            {
                if (a1 == -2005397136)
                    return "There is no master clock in the performance. Be sure to call IDirectMusicPerformance::Init().";
                switch (a1)
                {
                case -2005397199:
                    result = "The operation could not be completed because the software synth has not  yet been fully configured.";
                    break;
                case -2005397198:
                    result = "The operation cannot be carried out while the synthesizer is active.";
                    break;
                case -2005397197:
                    result = "An error occurred while attempting to read from the IStream* object.";
                    break;
                case -2005397196:
                    result = "The operation cannot be performed because the final instance of the DirectMusic object was released"
                        ". Ports cannot be used after final  release of the DirectMusic object.";
                    break;
                case -2005397195:
                    result = "There was no data in the referenced buffer.";
                    break;
                case -2005397194:
                    result = "There is insufficient space to insert the given event into the buffer.";
                    break;
                case -2005397193:
                    result = "The given operation could not be carried out because the port is a capture port.";
                    break;
                case -2005397192:
                    result = "The given operation could not be carried out because the port is a render port.";
                    break;
                case -2005397191:
                    result = "The port could not be created because no DirectSound has been specified. Specify a DirectSound inte"
                        "rface via the IDirectMusic::SetDirectSound method; pass NULL to have DirectMusic manage usage of DirectSound.";
                    break;
                case -2005397190:
                    result = "The operation cannot be carried out while the port is active.";
                    break;
                case -2005397189:
                    result = "Invalid DirectSound buffer was handed to port. ";
                    break;
                case -2005397188:
                    result = "Invalid buffer format was handed to the synth sink.";
                    break;
                case -2005397187:
                    result = "The operation cannot be carried out while the synthesizer is inactive.";
                    break;
                case -2005397186:
                    result = "IDirectMusic::SetDirectSound has already been called. It may not be changed while in use.";
                    break;
                case -2005397185:
                    result = "The given event is invalid (either it is not a valid MIDI message or it makes use of running status"
                        "). The event cannot be packed into the buffer.";
                    break;
                case -2005397168:
                    result = "The IStream* object does not contain data supported by the loading object.";
                    break;
                case -2005397167:
                    result = "The object has already been initialized.";
                    break;
                case -2005397166:
                    result = "The file does not contain a valid band.";
                    break;
                case -2005397163:
                    result = "The IStream* object's data does not have a track header as the first chunk, and therefore can not b"
                        "e read by the segment object.";
                    break;
                case -2005397162:
                    result = "The IStream* object's data does not have a tool header as the first chunk, and therefore can not be"
                        " read by the graph object.";
                    break;
                case -2005397161:
                    result = "The IStream* object's data contains an invalid track header (ckid is 0 and fccType is NULL,) and th"
                        "erefore can not be read by the segment object.";
                    break;
                case -2005397160:
                    result = "The IStream* object's data contains an invalid tool header (ckid is 0 and fccType is NULL,) and the"
                        "refore can not be read by the graph object.";
                    break;
                case -2005397159:
                    result = "The graph object was unable to load all tools from the IStream* object data. This may be due to err"
                        "ors in the stream, or the tools being incorrectly registered on the client.";
                    break;
                case -2005397152:
                    result = "The segment object was unable to load all tracks from the IStream* object data. This may be due to "
                        "errors in the stream, or the tracks being incorrectly registered on the client.";
                    break;
                case -2005397151:
                    result = "The object requested was not found (numerically equal to DMUS_E_NOT_FOUND)";
                    break;
                case -2005397150:
                    result = "A required object is not initialized or failed to initialize.";
                    break;
                case -2005397149:
                    result = "The requested parameter type is currently disabled. Parameter types may be enabled and disabled by "
                        "certain calls to SetParam().";
                    break;
                case -2005397148:
                    result = "The requested parameter type is not supported on the object.";
                    break;
                case -2005397147:
                    result = "The time is in the past, and the operation can not succeed.";
                    break;
                case -2005397146:
                    result = "The requested track is not contained by the segment.";
                    break;
                case -2005397145:
                    result = "The track does not support clock time playback or getparam.";
                    break;
                default:
                    return "n/a";
                }
                return result;
            }
            if (a1 <= -931722292)
            {
                if (a1 == -931722292)
                    return "The image file format is unrecognized";
                if (a1 > -2005396957)
                {
                    if (a1 > -931722312)
                    {
                        switch (a1)
                        {
                        case -931722311:
                            result = "A NULL pointer was passed as a parameter";
                            break;
                        case -931722310:
                            result = "The Device Index passed in is invalid";
                            break;
                        case -931722309:
                            result = "DirectDraw has not been created";
                            break;
                        case -931722308:
                            result = "Direct3D has not been created";
                            break;
                        case -931722307:
                            result = "Direct3D device has not been created";
                            break;
                        case -931722306:
                            result = "Primary surface has not been created";
                            break;
                        case -931722305:
                            result = "Z buffer has not been created";
                            break;
                        case -931722304:
                            result = "Backbuffer has not been created";
                            break;
                        case -931722303:
                            result = "Failed to update caps database after changing display mode";
                            break;
                        case -931722302:
                            result = "Could not create Z buffer";
                            break;
                        case -931722301:
                            result = "Display mode is not valid";
                            break;
                        case -931722300:
                            result = "One or more of the parameters passed is invalid";
                            break;
                        case -931722299:
                            result = "D3DX failed to initialize itself";
                            break;
                        case -931722298:
                            result = "D3DX failed to start up";
                            break;
                        case -931722297:
                            result = "D3DXInitialize() must be called first";
                            break;
                        case -931722296:
                            result = "D3DX is not initialized yet";
                            break;
                        case -931722295:
                            result = "Failed to render text to the surface";
                            break;
                        case -931722294:
                            result = "Bad D3DX context";
                            break;
                        case -931722293:
                            result = "The requested device capabilities are not supported";
                            break;
                        default:
                            return "n/a";
                        }
                    }
                    else if (a1 == -931722312)
                    {
                        return "Out of memory";
                    }
                    else
                    {
                        switch (a1)
                        {
                        case -2005396956:
                            result = "A script routine written in AudioVBScript failed because an invalid operation occurred.  For ex"
                                "ample, adding the number 3 to a segment object would produce this error.  So would attempting t"
                                "o call a routine that doesn't exist.";
                            break;
                        case -2005396955:
                            result = "A script routine written in AudioVBScript failed because a function outside of a script failed "
                                "to complete. For example, a call to PlaySegment that fails to play because of low memory would "
                                "return this error.";
                            break;
                        case -2005396954:
                            result = "The Performance has set up some PChannels using the AssignPChannel command, which makes it not "
                                "capable of supporting audio paths.";
                            break;
                        case -2005396953:
                            result = "This is the inverse of the previous error. The Performance has set up some audio paths, which m"
                                "akes is incompatible with the calls to allocate pchannels, etc. ";
                            break;
                        case -2005396952:
                            result = "A segment or song was asked for its embedded audio path configuration, but there isn't any. ";
                            break;
                        case -2005396951:
                            result = "An audiopath is inactive, perhaps because closedown was called.";
                            break;
                        case -2005396950:
                            result = "An audiopath failed to create because a requested buffer could not be created.";
                            break;
                        case -2005396949:
                            result = "An audiopath could not be used for playback because it lacked port assignments.";
                            break;
                        case -2005396948:
                            result = "Attempt was made to play segment in audiopath mode and there was no audiopath.";
                            break;
                        case -2005396947:
                            result = "Invalid data was found in a RIFF file chunk.";
                            break;
                        case -2005396946:
                            result = "Attempt was made to create an audiopath that sends to a global effects buffer which did not exist.";
                            break;
                        case -2005396945:
                            result = "The file does not contain a valid container object.";
                            break;
                        default:
                            return "n/a";
                        }
                    }
                }
                else
                {
                    if (a1 == -2005396957)
                        return "A script written in AudioVBScript could not be read because it contained a statement that is not allo"
                        "wed by the AudioVBScript language.";
                    if (a1 <= -2005396975)
                    {
                        if (a1 == -2005396975)
                            return "An attempt to use this object failed because it first needs to be loaded.";
                        if (a1 > -2005396991)
                        {
                            switch (a1)
                            {
                            case -2005396990:
                                return "Value is out of range, for instance the requested length is longer than the segment.";
                            case -2005396989:
                                return "Segment initialization failed, most likely due to a critical memory situation.";
                            case -2005396988:
                                return "The DMUS_PMSG has already been sent to the performance object via IDirectMusicPerformance::SendPMsg().";
                            case -2005396987:
                                return "The DMUS_PMSG was either not allocated by the performance via IDirectMusicPerformance::AllocPMs"
                                    "g(), or it was already freed via IDirectMusicPerformance::FreePMsg().";
                            case -2005396986:
                                return "The default system port could not be opened.";
                            case -2005396985:
                                return "A call to MIDIToMusic() or MusicToMIDI() resulted in an error because the requested conversion "
                                    "could not happen. This usually occurs when the provided DMUS_CHORD_KEY structure has an invalid"
                                    " chord or scale pattern.";
                            case -2005396976:
                                return "DMUS_E_DESCEND_CHUNK_FAIL is returned when the end of the file  was reached before the desired "
                                    "chunk was found.";
                            }
                        }
                        else
                        {
                            if (a1 == -2005396991)
                                return "The tool is already contained in the graph. Create a new instance.";
                            if (a1 > -2005397116)
                            {
                                switch (a1)
                                {
                                case -2005397115:
                                    return "Object was not found.";
                                case -2005397114:
                                    return "The file name is missing from the DMUS_OBJECTDESC.";
                                case -2005396992:
                                    return "The file requested is not a valid file.";
                                }
                            }
                            else
                            {
                                switch (a1)
                                {
                                case -2005397116:
                                    return "Unable to find or create object.";
                                case -2005397120:
                                    return "The class id field is required and missing in the DMUS_OBJECTDESC.";
                                case -2005397119:
                                    return "The requested file path is invalid.";
                                case -2005397118:
                                    return "File open failed - either file doesn't exist or is locked.";
                                case -2005397117:
                                    return "Search data type is not supported.";
                                }
                            }
                        }
                        return "n/a";
                    }
                    switch (a1)
                    {
                    case -2005396973:
                        result = "The activeX scripting engine for the script's language is not compatible with DirectMusic.";
                        break;
                    case -2005396972:
                        result = "A varient was used that had a type that is not supported by DirectMusic.";
                        break;
                    case -2005396971:
                        result = "An error was encountered while parsing or executing the script. The pErrorInfo parameter (if supp"
                            "lied) was filled with information about the error.";
                        break;
                    case -2005396970:
                        result = "Loading of oleaut32.dll failed.  VBScript and other activeX scripting languages require use of ol"
                            "eaut32.dll.  On platforms where oleaut32.dll is not present, only the DirectMusicScript language,"
                            " which doesn't require oleaut32.dll can be used.";
                        break;
                    case -2005396969:
                        result = "An error occured while parsing a script loaded using LoadScript.  The script that was loaded contains an error.";
                        break;
                    case -2005396968:
                        result = "The script file is invalid.";
                        break;
                    case -2005396967:
                        result = "The file contains an invalid script track.";
                        break;
                    case -2005396966:
                        result = "The script does not contain a variable with the specified name.";
                        break;
                    case -2005396965:
                        result = "The script does not contain a routine with the specified name.";
                        break;
                    case -2005396964:
                        result = "Scripts variables for content referenced or embedded in a script cannot be set.";
                        break;
                    case -2005396963:
                        result = "Attempt was made to set a script's variable by reference to a value that was not an object type.";
                        break;
                    case -2005396962:
                        result = "Attempt was made to set a script's variable by value to an object that does not support a default"
                            " value property.";
                        break;
                    case -2005396960:
                        result = "The file contains an invalid segment trigger track.";
                        break;
                    case -2005396959:
                        result = "The file contains an invalid lyrics track.";
                        break;
                    case -2005396958:
                        result = "The file contains an invalid parameter control track.";
                        break;
                    default:
                        return "n/a";
                    }
                }
                return result;
            }
            if (a1 <= 0)
            {
                if (!a1)
                    return "The function completed successfully";
                switch (a1)
                {
                case -931722291:
                    result = "The image file loading library error";
                    break;
                case -931722290:
                    result = "Could not obtain device caps";
                    break;
                case -931722289:
                    result = "Resize does not work for full-screen";
                    break;
                case -931722288:
                    result = "Resize does not work for non-windowed contexts";
                    break;
                case -931722287:
                    result = "Front buffer already exists";
                    break;
                case -931722286:
                    result = "The app is using the primary in full-screen mode";
                    break;
                case -931722285:
                    result = "Could not get device context";
                    break;
                case -931722284:
                    result = "Could not bitBlt";
                    break;
                case -931722283:
                    result = "There is no surface backing up this texture";
                    break;
                case -931722282:
                    result = "There is no such miplevel for this surface";
                    break;
                case -931722281:
                    result = "The surface is not paletted";
                    break;
                case -931722280:
                    result = "An error occured while enumerating surface formats";
                    break;
                case -931722279:
                    result = "D3DX only supports color depths of 16 bit or greater";
                    break;
                case -931722278:
                    result = "The file format is invalid";
                    break;
                case -931722277:
                    result = "No suitable match found";
                    break;
                default:
                    return "n/a";
                }
                return result;
            }
            if (a1 <= 262147)
            {
                if (a1 == 262147)
                    return "End of stream. Sample not updated.";
                switch (a1)
                {
                case 1:
                    result = "Call successful, but returned FALSE";
                    break;
                case 2:
                    return "The system cannot find the file specified.";
                case 3:
                LABEL_928:
                    result = "The system cannot find the path specified.";
                    break;
                case 4:
                LABEL_929:
                    result = "The system cannot open the file.";
                    break;
                case 5:
                    result = "Access is denied.";
                    break;
                case 6:
                    result = "The handle is invalid.";
                    break;
                case 8:
                LABEL_932:
                    result = "Not enough storage is available to process this command.";
                    break;
                case 9:
                LABEL_933:
                    result = "The storage control block address is invalid.";
                    break;
                case 10:
                LABEL_934:
                    result = "The environment is incorrect.";
                    break;
                case 11:
                LABEL_935:
                    result = "An attempt was made to load a program with an incorrect format.";
                    break;
                case 14:
                    result = "The system cannot find the drive specified.";
                    break;
                default:
                    return "n/a";
                }
                return result;
            }
            if (a1 <= 262796)
            {
                if (a1 == 262796)
                    return "The audio stream did not contain sufficient information to determine the contents of each channel.";
                if (a1 <= 262744)
                {
                    if (a1 == 262744)
                        return "Cannot play back the audio stream: no audio hardware is available.";
                    if (a1 > 262725)
                    {
                        switch (a1)
                        {
                        case 262726:
                            return "Some connections have failed and have been deferred.";
                        case 262736:
                            return "The resource specified is no longer needed.";
                        case 262740:
                            return "A connection could not be made with the media type in the persistent graph, but has been made wit"
                                "h a negotiated media type.";
                        case 262743:
                            return "Cannot play back the video stream: no suitable decompressor could be found.";
                        }
                    }
                    else
                    {
                        switch (a1)
                        {
                        case 262725:
                            return "The file contained some property settings that were not used.";
                        case 262403:
                            return "The end of the list has been reached.";
                        case 262701:
                            return "An attempt to add a filter with a duplicate name succeeded with a modified name.";
                        case 262711:
                            return "The state transition has not completed.";
                        case 262722:
                            return "Some of the streams in this movie are in an unsupported format.";
                        }
                    }
                    return "n/a";
                }
                if (a1 > 262760)
                {
                    switch (a1)
                    {
                    case 262768:
                        return "The stop time for the sample was not set.";
                    case 262782:
                        return "There was no preview pin available, so the capture pin output is being split to provide both capture and preview.";
                    case 262784:
                        return "The current title was not a sequential set of chapters (PGC), and the returned timing information m"
                            "ight not be continuous.";
                    }
                    return "n/a";
                }
                if (a1 == 262760)
                    return "The graph can't be cued because of lack of or corrupt data.";
                if (a1 != 262746)
                {
                    switch (a1)
                    {
                    case 262752:
                        return "The value returned had to be estimated.  It's accuracy can not be guaranteed.";
                    case 262755:
                        return "This success code is reserved for internal purposes within ActiveMovie.";
                    case 262759:
                        return "The stream has been turned off.";
                    }
                    return "n/a";
                }
                return "Cannot play back the video stream: format 'RPZA' is not supported.";
            }
            if (a1 <= 142086657)
            {
                if (a1 == 142086657)
                    return "Return value from IDirectMusicTool::ProcessPMsg() which indicates to the performance that it should fre"
                    "e the PMsg automatically.";
                if (a1 > 141953135)
                {
                    switch (a1)
                    {
                    case 142082058:
                        return "The call succeeded, but we had to substitute the 3D algorithm";
                    case 142086289:
                        return "The object could only load partially. This can happen if some components are not registered properl"
                            "y, such as embedded tracks and tools. This can also happen if some content is missing. For example,"
                            " if a segment uses a DLS collection that is not in the loader's current search directory.";
                    case 142086290:
                        return "Return value from IDirectMusicBand::Download() which indicates that some of the instruments safely "
                            "downloaded, but others failed. This usually occurs when some instruments are on PChannels not suppo"
                            "rted by the performance or port.";
                    case 142086656:
                        return "Return value from IDirectMusicTool::ProcessPMsg() which indicates to the performance that it should"
                            " cue the PMsg again automatically.";
                    }
                }
                else
                {
                    switch (a1)
                    {
                    case 141953135:
                        return "The call succeeded but there won't be any mipmaps generated";
                    case 262797:
                        return "The seek into the movie was not frame accurate.";
                    case 1376261:
                        return "Full duplex";
                    case 1376266:
                        return "Half duplex";
                    case 1376272:
                        return "Pending";
                    }
                }
                return "n/a";
            }
            if (a1 <= 142086675)
            {
                switch (a1)
                {
                case 142086675:
                    return "Returned from IDirectMusicPerformance::MIDIToMusic(),  and IDirectMusicPerformance::MusicToMIDI(), th"
                        "is indicates  that the note conversion generated a note value that is below 0,  so it has been bumped"
                        " up one or more octaves to be in the proper MIDI range of 0 through 127.  Note that this is valid for"
                        " MIDIToMusic() when using play modes DMUS_PLAYMODE_FIXEDTOCHORD and DMUS_PLAYMODE_FIXEDTOKEY, both of"
                        " which store MIDI values in wMusicValue. With MusicToMIDI(), it is valid for all play modes. Ofcourse"
                        ", DMUS_PLAYMODE_FIXED will never return this success code.";
                case 142086658:
                    return "Return value from IDirectMusicTrack::Play() which indicates to the segment that the track has no more"
                        " data after mtEnd.";
                case 142086672:
                    return "Returned string has been truncated to fit the buffer size.";
                case 142086673:
                    return "Returned from IDirectMusicGraph::StampPMsg(), this indicates that the PMsg is already stamped with th"
                        "e last tool in the graph. The returned PMsg's tool pointer is now NULL.";
                case 142086674:
                    return "Returned from IDirectMusicPerformance::MusicToMIDI(), this indicates  that no note has been calculate"
                        "d because the music value has the note  at a position higher than the top note of the chord. This app"
                        "lies only to DMUS_PLAYMODE_NORMALCHORD play mode. This success code indicates that the caller should "
                        "not do anything with the note. It is not meant to be played against this chord.";
                }
                return "n/a";
            }
            switch (a1)
            {
            case 142086676:
                return "Returned from IDirectMusicPerformance::MIDIToMusic(),  and IDirectMusicPerformance::MusicToMIDI(), this"
                    " indicates  that the note conversion generated a note value that is above 127, so it has been bumped do"
                    "wn one or more octaves to be in the proper MIDI range of 0 through 127.  Note that this is valid for MI"
                    "DIToMusic() when using play modes DMUS_PLAYMODE_FIXEDTOCHORD and DMUS_PLAYMODE_FIXEDTOKEY, both of whic"
                    "h store MIDI values in wMusicValue. With MusicToMIDI(), it is valid for all play modes. Ofcourse, DMUS_"
                    "PLAYMODE_FIXED will never return this success code.";
            case 142086677:
                return "Although the audio output from the port will be routed to the same device as the given DirectSound buff"
                    "er, buffer controls such as pan and volume will not affect the output.";
            case 142086678:
                return "The requested operation was not performed because during CollectGarbage the loader determined that the "
                    "object had been released.";
            }
            return "n/a";
        }
        if (a1 == -2005397200)
            return "An attempt was made to close the software synthesizer while it was already  open.";
        if (a1 <= -2005531762)
        {
            if (a1 == -2005531762)
                return "Bad file float size";
            if (a1 > -2005532083)
            {
                if (a1 > -2005531973)
                {
                    switch (a1)
                    {
                    case -2005531804:
                        return "Bad array size";
                    case -2005531803:
                        return "Bad data reference";
                    case -2005531802:
                        result = "Internal error";
                        break;
                    case -2005531801:
                        return "No more objects";
                    case -2005531800:
                        result = "Bad intrinsics";
                        break;
                    case -2005531799:
                        result = "No more stream handles";
                        break;
                    case -2005531798:
                        return "No more data";
                    case -2005531797:
                        return "Bad cache file";
                    case -2005531796:
                        result = "No internet";
                        break;
                    case -2005531772:
                        result = "Bad object";
                        break;
                    case -2005531771:
                        result = "Bad value";
                        break;
                    case -2005531770:
                        result = "Bad type";
                        break;
                    case -2005531769:
                        return "Not found";
                    case -2005531768:
                        result = "Not done yet";
                        break;
                    case -2005531767:
                        result = "File not found";
                        break;
                    case -2005531766:
                        result = "Resource not found";
                        break;
                    case -2005531765:
                        result = "Bad resource";
                        break;
                    case -2005531764:
                        result = "Bad file type";
                        break;
                    case -2005531763:
                        result = "Bad file version";
                        break;
                    default:
                        return "n/a";
                    }
                }
                else
                {
                    if (a1 == -2005531973)
                        return "Surfaces created by one direct draw device cannot be used directly by another direct draw device.";
                    if (a1 <= -2005532012)
                    {
                        if (a1 == -2005532012)
                            return "The attempt to page unlock a surface failed.";
                        if (a1 > -2005532070)
                        {
                            switch (a1)
                            {
                            case -2005532069:
                                return "Attempt was made to set a palette on a mipmap sublevel";
                            case -2005532052:
                                return "A DC has already been returned for this surface. Only one DC can be retrieved per surface.";
                            case -2005532042:
                                return "An attempt was made to allocate non-local video memory from a device that does not support non-"
                                    "local video memory.";
                            case -2005532032:
                                return "The attempt to page lock a surface failed.";
                            }
                        }
                        else
                        {
                            switch (a1)
                            {
                            case -2005532070:
                                return "Attempt was made to create or set a device window without first setting the focus window";
                            case -2005532082:
                                return "The display is currently in an unsupported mode";
                            case -2005532081:
                                return "Operation could not be carried out because there is no mip-map texture mapping hardware present or available.";
                            case -2005532080:
                                return "The requested action could not be performed because the surface was of the wrong type.";
                            case -2005532072:
                                return "Device does not support optimized surfaces, therefore no video memory optimized surfaces";
                            case -2005532071:
                                return "Surface is an optimized surface, but has not yet been allocated any memory";
                            }
                        }
                        return "n/a";
                    }
                    switch (a1)
                    {
                    case -2005531992:
                        result = "An attempt was made to page unlock a surface with no outstanding page locks.";
                        break;
                    case -2005531982:
                        result = "There is more data available than the specified buffer size could hold";
                        break;
                    case -2005531981:
                        result = "The data has expired and is therefore no longer valid.";
                        break;
                    case -2005531980:
                        result = "The mode test has finished executing.";
                        break;
                    case -2005531979:
                        result = "The mode test has switched to a new mode.";
                        break;
                    case -2005531978:
                        result = "D3D has not yet been initialized.";
                        break;
                    case -2005531977:
                        result = "The video port is not active";
                        break;
                    case -2005531976:
                        result = "The monitor does not have EDID data.";
                        break;
                    case -2005531975:
                        result = "The driver does not enumerate display mode refresh rates.";
                        break;
                    default:
                        return "n/a";
                    }
                }
            }
            else if (a1 == -2005532083)
            {
                return "The surface being used is not a palette-based surface";
            }
            else
            {
                switch (a1)
                {
                case -2005532222:
                    result = "Access to this surface is being refused because the surface is gone. The DIRECTDRAWSURFACE object r"
                        "epresenting this surface should have Restore called on it.";
                    break;
                case -2005532212:
                    result = "The requested surface is not attached.";
                    break;
                case -2005532202:
                    result = "Height requested by DirectDraw is too large.";
                    break;
                case -2005532192:
                    result = "Size requested by DirectDraw is too large --  The individual height and width are OK.";
                    break;
                case -2005532182:
                    result = "Width requested by DirectDraw is too large.";
                    break;
                case -2005532162:
                    result = "Pixel format requested is unsupported by DirectDraw";
                    break;
                case -2005532152:
                    result = "Bitmask in the pixel format requested is unsupported by DirectDraw";
                    break;
                case -2005532151:
                    result = "The specified stream contains invalid data";
                    break;
                case -2005532135:
                    result = "vertical blank is in progress";
                    break;
                case -2005532132:
                    result = "Was still drawing";
                    break;
                case -2005532130:
                    result = "The specified surface type requires specification of the COMPLEX flag";
                    break;
                case -2005532112:
                    result = "Rectangle provided was not horizontally aligned on reqd. boundary";
                    break;
                case -2005532111:
                    result = "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier.";
                    break;
                case -2005532110:
                    result = "A DirectDraw object representing this driver has already been created for this process.";
                    break;
                case -2005532109:
                    result = "A hardware only DirectDraw object creation was attempted but the driver did not support any hardware.";
                    break;
                case -2005532108:
                    result = "this process already has created a primary surface";
                    break;
                case -2005532107:
                    result = "software emulation not available.";
                    break;
                case -2005532106:
                    result = "region passed to Clipper::GetClipList is too small.";
                    break;
                case -2005532105:
                    result = "an attempt was made to set a clip list for a clipper objec that is already monitoring an hwnd.";
                    break;
                case -2005532104:
                    result = "No clipper object attached to surface object";
                    break;
                case -2005532103:
                    result = "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND.";
                    break;
                case -2005532102:
                    result = "HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state.";
                    break;
                case -2005532101:
                    result = "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces "
                        "or palettes created.";
                    break;
                case -2005532100:
                    result = "No palette object attached to this surface.";
                    break;
                case -2005532099:
                    result = "No hardware support for 16 or 256 color palettes.";
                    break;
                case -2005532098:
                    result = "If a clipper object is attached to the source surface passed into a BltFast call.";
                    break;
                case -2005532097:
                    result = "No blter.";
                    break;
                case -2005532096:
                    result = "No DirectDraw ROP hardware.";
                    break;
                case -2005532095:
                    result = "returned when GetOverlayPosition is called on a hidden overlay";
                    break;
                case -2005532094:
                    result = "returned when GetOverlayPosition is called on a overlay that UpdateOverlay has never been called on"
                        " to establish a destionation.";
                    break;
                case -2005532093:
                    result = "returned when the position of the overlay on the destionation is no longer legal for that destionation.";
                    break;
                case -2005532092:
                    result = "returned when an overlay member is called for a non-overlay surface";
                    break;
                case -2005532091:
                    result = "An attempt was made to set the cooperative level when it was already set to exclusive.";
                    break;
                case -2005532090:
                    result = "An attempt has been made to flip a surface that is not flippable.";
                    break;
                case -2005532089:
                    result = "Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created.";
                    break;
                case -2005532088:
                    result = "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this proc"
                        "ess, has been attempted.";
                    break;
                case -2005532087:
                    result = "Windows can not create any more DCs, or a DC was requested for a paltte-indexed surface when the su"
                        "rface had no palette AND the display mode was not palette-indexed (in this case DirectDraw cannot s"
                        "elect a proper palette into the DC)";
                    break;
                case -2005532086:
                    result = "No DC was ever created for this surface.";
                    break;
                case -2005532085:
                    result = "This surface can not be restored because it was created in a different mode.";
                    break;
                case -2005532084:
                    result = "This surface can not be restored because it is an implicitly created surface.";
                    break;
                default:
                    return "n/a";
                }
            }
            return result;
        }
        if (a1 <= -2005401500)
        {
            if (a1 == -2005401500)
                return "The specified WAVE format is not supported";
            if (a1 > -2005530521)
            {
                if (a1 > -2005529769)
                {
                    if (a1 > -2005529764)
                    {
                        switch (a1)
                        {
                        case -2005401590:
                            return "The call failed because resources (such as a priority level) were already being used by another caller";
                        case -2005401570:
                            return "The control (vol, pan, etc.) requested by the caller is not available";
                        case -2005401550:
                            return "This call is not valid for the current state of this object";
                        case -2005401530:
                            return "The caller does not have the priority level required for the function to succeed";
                        }
                        return "n/a";
                    }
                    switch (a1)
                    {
                    case -2005529764:
                        return "Can Not remove last item";
                    case -2005529768:
                        return "Too many influences";
                    case -2005529767:
                        return "Invalid data";
                    case -2005529766:
                        return "Loaded mesh has no data";
                    default:
                        return "Duplicate named fragment";
                    }
                }
                else
                {
                    if (a1 == -2005529769)
                        return "Skinning not supported";
                    if (a1 > -2005530516)
                    {
                        switch (a1)
                        {
                        case -2005530515:
                            return "Driver invalid call";
                        case -2005529772:
                            return "Can not modify index buffer";
                        case -2005529771:
                            return "Invalid mesh";
                        case -2005529770:
                            return "Cannot attr sort";
                        }
                        return "n/a";
                    }
                    switch (a1)
                    {
                    case -2005530516:
                        return "Invalid call";
                    case -2005530520:
                        return "Device lost";
                    case -2005530519:
                        return "Device not reset";
                    case -2005530518:
                        return "Not available";
                    default:
                        return "Invalid device";
                    }
                }
            }
            else
            {
                if (a1 == -2005530521)
                    return "More data";
                if (a1 > -2005530597)
                {
                    if (a1 > -2005530591)
                    {
                        switch (a1)
                        {
                        case -2005530590:
                            return "Unsupported texture filter";
                        case -2005530586:
                            return "Conflicting texture palette";
                        case -2005530585:
                            return "Driver internal error";
                        case -2005530522:
                            return "Not found";
                        }
                    }
                    else
                    {
                        switch (a1)
                        {
                        case -2005530591:
                            return "Conflicting render state";
                        case -2005530596:
                            return "Unsupported alpha arg";
                        case -2005530595:
                            return "Too many operations";
                        case -2005530594:
                            return "Conflicting texture filter";
                        case -2005530593:
                            return "Unsupported factor value";
                        }
                    }
                    return "n/a";
                }
                if (a1 == -2005530597)
                    return "Unsupported alpha operation";
                if (a1 > -2005531756)
                {
                    switch (a1)
                    {
                    case -2005531755:
                        return "Bad cache file";
                    case -2005530600:
                        return "Wrong texture format";
                    case -2005530599:
                        return "Unsupported color operation";
                    case -2005530598:
                        return "Unsupported color arg";
                    }
                    return "n/a";
                }
                switch (a1)
                {
                case -2005531756:
                    return "No more data";
                case -2005531761:
                    return "Bad file";
                case -2005531760:
                    return "Parse error";
                case -2005531759:
                    return "Bad array size";
                case -2005531758:
                    return "Bad data reference";
                default:
                    return "No more objects";
                }
            }
        }
        if (a1 > -2005397227)
        {
            switch (a1)
            {
            case -2005397226:
                result = "The IStream* doesn't support Write().";
                break;
            case -2005397225:
                result = "The RIFF parser doesn't contain a required chunk while parsing file.";
                break;
            case -2005397223:
                result = "Invalid download id was used in the process of creating a download buffer.";
                break;
            case -2005397216:
                result = "Tried to unload an object that was not downloaded or previously unloaded.";
                break;
            case -2005397215:
                result = "Buffer was already downloaded to synth.";
                break;
            case -2005397214:
                result = "The specified property item was not recognized by the target object.";
                break;
            case -2005397213:
                result = "The specified property item may not be set on the target object.";
                break;
            case -2005397212:
                result = "* The specified property item may not be retrieved from the target object.";
                break;
            case -2005397211:
                result = "Wave chunk has more than one interleaved channel. DLS format requires MONO.";
                break;
            case -2005397210:
                result = "Invalid articulation chunk in DLS collection.";
                break;
            case -2005397209:
                result = "Invalid instrument chunk in DLS collection.";
                break;
            case -2005397208:
                result = "Wavelink chunk in DLS collection points to invalid wave.";
                break;
            case -2005397207:
                result = "Articulation missing from instrument in DLS collection.";
                break;
            case -2005397206:
                result = "Downoaded DLS wave is not in PCM format. ";
                break;
            case -2005397205:
                result = "Bad wave chunk in DLS collection";
                break;
            case -2005397204:
                result = "Offset Table for download buffer has errors. ";
                break;
            case -2005397203:
                result = "Attempted to download unknown data type.";
                break;
            case -2005397202:
                result = "The operation could not be completed because no sink was connected to the synthesizer.";
                break;
            case -2005397201:
                result = "An attempt was made to open the software synthesizer while it was already  open.";
                break;
            default:
                return "n/a";
            }
            return result;
        }
        if (a1 == -2005397227)
            return "The IStream* doesn't support Seek().";
        if (a1 > -2005397246)
        {
            switch (a1)
            {
            case -2005397245:
                result = "The requested device is already in use (possibly by a non-DirectMusic client) and cannot be opened again.";
                break;
            case -2005397244:
                result = "Buffer is not large enough for requested operation.";
                break;
            case -2005397243:
                result = "No buffer was prepared for the download data.";
                break;
            case -2005397242:
                result = "Download failed due to inability to access or create download buffer.";
                break;
            case -2005397240:
                result = "Error parsing DLS collection. File is corrupt.";
                break;
            case -2005397239:
                result = "Wave chunks in DLS collection file are at incorrect offsets.";
                break;
            case -2005397231:
                result = "Second attempt to load a DLS collection that is currently open. ";
                break;
            case -2005397229:
                result = "Error reading wave data from DLS collection. Indicates bad file.";
                break;
            case -2005397228:
                result = "There is no instrument in the collection that matches patch number.";
                break;
            default:
                return "n/a";
            }
            return result;
        }
        if (a1 == -2005397246)
            return "The requested operation cannot be performed while there are  instantiated ports in any process in the system.";
        if (a1 > -2005401420)
        {
            switch (a1)
            {
            case -2005401410:
                return "Attempt to use DirectSound 8 functionality on an older DirectSound object";
            case -2005401400:
                return "A circular loop of send effects was detected";
            case -2005401390:
                return "The GUID specified in an audiopath file does not match a valid MIXIN buffer";
            case -2005397247:
                return "An unexpected error was returned from a device driver, indicating possible failure of the driver or hardware.";
            }
            return "n/a";
        }
        if (a1 == -2005401420)
            return "Tried to create a DSBCAPS_CTRLFX buffer shorter than DSBSIZE_FX_MIN milliseconds";
        if (a1 == -2005401480)
            return "No sound driver is available for use";
        if (a1 != -2005401470)
        {
            if (a1 == -2005401450)
                return "The buffer memory has been lost, and must be restored";
            if (a1 == -2005401440)
                return "Another app has a higher priority level, preventing this call from succeeding";
            if (a1 != -2005401430)
                return "n/a";
            return "This object has not been initialized";
        }
        return "This object is already initialized";
    }
    if (a1 == -2005532232)
        return "Access to Surface refused because Surface is obscured.";
    if (a1 > -2147023743)
    {
        if (a1 > -2146073776)
        {
            if (a1 > -2005532542)
            {
                if (a1 > -2005532392)
                {
                    if (a1 > -2005532312)
                    {
                        if (a1 > -2005532272)
                        {
                            switch (a1)
                            {
                            case -2005532262:
                                return "This surface is already attached to the surface it is being attached to.";
                            case -2005532252:
                                return "This surface is already a dependency of the surface it is being made a dependency of.";
                            case -2005532242:
                                return "Access to this surface is being refused because the surface is already locked by another thread.";
                            case -2005532237:
                                return "Access to this surface is being refused because no driver exists which can supply a pointer to "
                                    "the surface. This is most likely to happen when attempting to lock the primary surface when no "
                                    "DCI provider is present. Will also happen on attempts to lock an optimized surface.";
                            }
                        }
                        else
                        {
                            switch (a1)
                            {
                            case -2005532272:
                                return "No src color key specified for this operation.";
                            case -2005532292:
                                return "Out of video memory";
                            case -2005532290:
                                return "hardware does not support clipped overlays";
                            case -2005532288:
                                return "Can only have ony color key active at one time for overlays";
                            case -2005532285:
                                return "Access to this palette is being refused because the palette is already locked by another thread.";
                            }
                        }
                    }
                    else
                    {
                        if (a1 == -2005532312)
                            return "The hardware needed for the requested operation has already been allocated.";
                        if (a1 > -2005532352)
                        {
                            switch (a1)
                            {
                            case -2005532342:
                                return "Operation could not be carried out because there is no texture mapping hardware present or available.";
                            case -2005532337:
                                return "Operation could not be carried out because there is no hardware support for vertical blank sync"
                                    "hronized operations.";
                            case -2005532332:
                                return "Operation could not be carried out because there is no hardware support for zbuffer blting.";
                            case -2005532322:
                                return "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not s"
                                    "upport z layering of overlays.";
                            }
                        }
                        else
                        {
                            switch (a1)
                            {
                            case -2005532352:
                                return "DirectDraw Surface is not in 8 bit color mode and the requested operation requires 8 bit color.";
                            case -2005532382:
                                return "Operation could not be carried out because there is no rotation hardware present or available.";
                            case -2005532362:
                                return "Operation could not be carried out because there is no hardware support for stretching";
                            case -2005532356:
                                return "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette.";
                            case -2005532355:
                                return "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bi"
                                    "t color index palette.";
                            }
                        }
                    }
                }
                else
                {
                    if (a1 == -2005532392)
                        return "Operation could not be carried out because there is no appropriate raster op hardware present or available.";
                    if (a1 > -2005532457)
                    {
                        if (a1 > -2005532432)
                        {
                            switch (a1)
                            {
                            case -2005532422:
                                return "Operation could not be carried out because there is no hardware present or available.";
                            case -2005532417:
                                return "Requested item was not found";
                            case -2005532412:
                                return "Operation could not be carried out because there is no overlay hardware present or available.";
                            case -2005532402:
                                return "Operation could not be carried out because the source and destination rectangles are on the sam"
                                    "e surface and overlap each other.";
                            }
                        }
                        else
                        {
                            switch (a1)
                            {
                            case -2005532432:
                                return "There is no GDI present.";
                            case -2005532452:
                                return "Operation could not be carried out because there is no hardware support of the dest color key.";
                            case -2005532450:
                                return "No DirectDraw support possible with current display driver";
                            case -2005532447:
                                return "Operation requires the application to have exclusive mode but the application does not have exclusive mode.";
                            case -2005532442:
                                return "Flipping visible surfaces is not supported.";
                            }
                        }
                    }
                    else
                    {
                        if (a1 == -2005532457)
                            return "Surface doesn't currently have a color key";
                        if (a1 > -2005532491)
                        {
                            switch (a1)
                            {
                            case -2005532490:
                                return "Operation could not be carried out because there is no hardware present which supports stereo surfaces";
                            case -2005532467:
                                return "no clip list available";
                            case -2005532462:
                                return "Operation could not be carried out because there is no color conversion hardware present or available.";
                            case -2005532460:
                                return "Create function called without DirectDraw object method SetCooperativeLevel being called.";
                            }
                        }
                        else
                        {
                            switch (a1)
                            {
                            case -2005532491:
                                return "Operation could not be carried out because there is no stereo hardware present or available.";
                            case -2005532527:
                                return "pixel format was invalid as specified";
                            case -2005532522:
                                return "Rectangle provided was invalid.";
                            case -2005532512:
                                return "Operation could not be carried out because one or more surfaces are locked";
                            case -2005532502:
                                return "There is no 3D present.";
                            case -2005532492:
                                return "Operation could not be carried out because there is no alpha accleration hardware present or available.";
                            }
                        }
                    }
                }
                return "n/a";
            }
            if (a1 == -2005532542)
                return "DirectDraw received a pointer that was an invalid DIRECTDRAW object.";
            if (a1 > -2146073248)
            {
                if (a1 > -2005532667)
                {
                    if (a1 > -2005532582)
                    {
                        switch (a1)
                        {
                        case -2005532577:
                            return "Unable to match primary surface creation request with existing primary surface.";
                        case -2005532572:
                            return "One or more of the caps bits passed to the callback are incorrect.";
                        case -2005532562:
                            return "DirectDraw does not support provided Cliplist.";
                        case -2005532552:
                            return "DirectDraw does not support the requested mode";
                        }
                    }
                    else
                    {
                        switch (a1)
                        {
                        case -2005532582:
                            return "Height of rectangle provided is not a multiple of reqd alignment";
                        case -2005532662:
                            return "This surface can not be attached to the requested surface.";
                        case -2005532652:
                            return "This surface can not be detached from the requested surface.";
                        case -2005532632:
                            return "Support is currently not available.";
                        case -2005532617:
                            return "An exception was encountered while performing the requested operation";
                        }
                    }
                    return "n/a";
                }
                if (a1 != -2005532667)
                {
                    if (a1 > -2146073072)
                    {
                        switch (a1)
                        {
                        case -2146073056:
                            return "Table full";
                        case -2146073040:
                            return "Timed out";
                        case -2146073024:
                            return "Uninitialized";
                        case -2146073008:
                            return "User cancel";
                        }
                    }
                    else
                    {
                        switch (a1)
                        {
                        case -2146073072:
                            return "Session full";
                        case -2146073232:
                            return "Player lost";
                        case -2146073216:
                            return "Player not in group";
                        case -2146073200:
                            return "Player not reachable";
                        case -2146073088:
                            return "Send too large";
                        }
                    }
                    return "n/a";
                }
                return "This object is already initialized";
            }
            if (a1 == -2146073248)
                return "Player already in group";
            if (a1 <= -2146073504)
            {
                if (a1 == -2146073504)
                    return "Invalid version";
                if (a1 > -2146073584)
                {
                    if (a1 != -2146073568)
                    {
                        switch (a1)
                        {
                        case -2146073552:
                            return "Invalid priority";
                        case -2146073536:
                            return "Invalid string";
                        case -2146073520:
                            return "Invalid url";
                        }
                        return "n/a";
                    }
                    return "Invalid player";
                }
                switch (a1)
                {
                case -2146073584:
                    return "Invalid password";
                case -2146073760:
                    return "Invalid handle";
                case -2146073744:
                    return "Invalid host address";
                case -2146073728:
                    return "Invalid instance";
                case -2146073712:
                    return "Invalid interface";
                }
                if (a1 != -2146073600)
                    return "n/a";
                return "Invalid object";
            }
            if (a1 <= -2146073328)
            {
                switch (a1)
                {
                case -2146073328:
                    return "No response";
                case -2146073488:
                    return "No caps";
                case -2146073472:
                    return "No connection";
                case -2146073456:
                    return "No host player";
                case -2146073344:
                    return "No more address components";
                }
                return "n/a";
            }
            if (a1 != -2146073312)
            {
                switch (a1)
                {
                case -2146073296:
                    return "Not host";
                case -2146073280:
                    return "Not ready";
                case -2146073264:
                    return "Not registered";
                }
                return "n/a";
            }
        }
        else
        {
            if (a1 == -2146073776)
                return "Invalid group";
            if (a1 > -2146106998)
            {
                if (a1 > -2146074256)
                {
                    if (a1 <= -2146074032)
                    {
                        if (a1 == -2146074032)
                            return "Hosting";
                        if (a1 <= -2146074112)
                        {
                            switch (a1)
                            {
                            case -2146074112:
                                return "End point not receiving";
                            case -2146074251:
                                return "Data too large";
                            case -2146074240:
                                return "Does not exist";
                            case -2146074235:
                                return "dpnsvr not available";
                            case -2146074224:
                                return "Duplicate command";
                            }
                            return "n/a";
                        }
                        if (a1 == -2146074096)
                            return "Enum query too large";
                        if (a1 == -2146074080)
                            return "Enum response too large";
                        if (a1 != -2146074064)
                        {
                            if (a1 == -2146074048)
                                return "Group not empty";
                            return "n/a";
                        }
                        return "Exception";
                    }
                    if (a1 <= -2146073856)
                    {
                        switch (a1)
                        {
                        case -2146073856:
                            return "Invalid application";
                        case -2146074016:
                            return "Host rejected connection";
                        case -2146074000:
                            return "Host terminated session";
                        case -2146073984:
                            return "Incomplete address";
                        case -2146073968:
                            return "Invalid address format";
                        }
                        return "n/a";
                    }
                    switch (a1)
                    {
                    case -2146073840:
                        return "Invalid command";
                    case -2146073824:
                        return "Invalid device address";
                    case -2146073808:
                        return "Invalid end point";
                    }
                    if (a1 != -2146073792)
                        return "n/a";
                    return "Invalid flags";
                }
                if (a1 == -2146074256)
                    return "Conversion";
                if (a1 <= -2146074512)
                {
                    if (a1 == -2146074512)
                        return "Already disconnecting";
                    if (a1 > -2146106992)
                    {
                        switch (a1)
                        {
                        case -2146074576:
                            return "Aborted";
                        case -2146074560:
                            return "Addressing";
                        case -2146074544:
                            return "Already closing";
                        case -2146074528:
                            return "Already connected";
                        }
                        return "n/a";
                    }
                    switch (a1)
                    {
                    case -2146106992:
                        return "Locked buffer";
                    case -2146106997:
                        return "Transport no session";
                    case -2146106996:
                        return "Transport no player";
                    case -2146106995:
                        return "User back";
                    case -2146106994:
                        return "No rec vol available";
                    }
                    return "Invalid buffer";
                }
                if (a1 <= -2146074336)
                {
                    switch (a1)
                    {
                    case -2146074336:
                        return "Cant create group";
                    case -2146074496:
                        return "Already initialized";
                    case -2146074480:
                        return "Already registered";
                    }
                    if (a1 != -2146074368)
                    {
                        if (a1 == -2146074352)
                            return "Can not cancel";
                        return "n/a";
                    }
                    return "Buffer too small";
                }
                switch (a1)
                {
                case -2146074320:
                    return "Cant create player";
                case -2146074304:
                    return "Cant launch application";
                case -2146074288:
                    return "Connecting";
                }
                if (a1 != -2146074272)
                    return "n/a";
                return "Connection lost";
            }
            if (a1 == -2146106998)
                return "Transport not init";
            if (a1 > -2146107022)
            {
                switch (a1)
                {
                case -2146107021:
                    result = "Already pending";
                    break;
                case -2146107020:
                    result = "Sound init failure";
                    break;
                case -2146107019:
                    result = "Time out";
                    break;
                case -2146107018:
                    result = "Connect aborted";
                    break;
                case -2146107017:
                    result = "No 3d sound";
                    break;
                case -2146107016:
                    result = "Already buffered";
                    break;
                case -2146107015:
                    result = "Not buffered";
                    break;
                case -2146107014:
                    return "Hosting";
                case -2146107013:
                    result = "Not hosting";
                    break;
                case -2146107012:
                    return "Invalid device";
                case -2146107011:
                    result = "Record system error";
                    break;
                case -2146107010:
                    result = "Playback system error";
                    break;
                case -2146107009:
                    result = "Send error";
                    break;
                case -2146107008:
                    return "User cancel";
                case -2146107005:
                    result = "Run setup";
                    break;
                case -2146107004:
                    result = "Incompatible version";
                    break;
                case -2146107001:
                    result = "Initialized";
                    break;
                case -2146107000:
                    result = "No transport";
                    break;
                case -2146106999:
                    result = "No callback";
                    break;
                default:
                    return "n/a";
                }
                return result;
            }
            if (a1 == -2146107022)
                return "Compression not supported";
            if (a1 <= -2146107092)
            {
                if (a1 == -2146107092)
                    return "Session lost";
                if (a1 <= -2146107272)
                {
                    if (a1 != -2146107272)
                    {
                        if (a1 == -2147023728)
                            return "The specified property ID is not supported for the specified property set.";
                        if (a1 == -2147023726)
                            return "The specified property set is not supported.";
                        if (a1 != -2147023649)
                        {
                            if (a1 != -2146107362)
                            {
                                if (a1 == -2146107318)
                                    return "Exception";
                                return "n/a";
                            }
                            return "Buffer too small";
                        }
                        return "This object is already initialized";
                    }
                    return "Invalid flags";
                }
                if (a1 != -2146107262)
                {
                    if (a1 != -2146107257)
                    {
                        if (a1 != -2146107247)
                        {
                            if (a1 != -2146107242)
                                return "n/a";
                            return "Invalid handle";
                        }
                        return "Invalid group";
                    }
                    return "Invalid player";
                }
                return "Invalid object";
            }
            if (a1 <= -2146107029)
            {
                if (a1 == -2146107029)
                    return "Not connected";
                if (a1 == -2146107090)
                    return "No voice session";
                if (a1 != -2146107032)
                {
                    if (a1 == -2146107031)
                        return "Not initialized";
                    if (a1 == -2146107030)
                        return "Connected";
                    return "n/a";
                }
                return "Connection lost";
            }
            if (a1 == -2146107026)
                return "Connect aborting";
            if (a1 != -2146107025)
            {
                if (a1 == -2146107024)
                    return "Invalid target";
                if (a1 == -2146107023)
                    return "Transport not host";
                return "n/a";
            }
        }
        return "Not allowed";
    }
    if (a1 == -2147023743)
        return "The application was written for an unsupported prerelease version of DirectInput.";
    if (a1 > -2147220903)
    {
        if (a1 <= -2147220736)
        {
            if (a1 == -2147220736)
                return "Device driver-specific codes. Unless the specific driver has been precisely identified, no meaning should"
                " be attributed to these values other than that the driver originated the error.";
            switch (a1)
            {
            case -2147220901:
                result = "ActiveMovie cannot play MPEG movies on this processor.";
                break;
            case -2147220900:
                result = "Cannot play back the audio stream: the audio format is not supported.";
                break;
            case -2147220899:
                result = "Cannot play back the video stream: the video format is not supported.";
                break;
            case -2147220898:
                result = "ActiveMovie cannot play this video stream because it falls outside the constrained standard.";
                break;
            case -2147220897:
                result = "Cannot perform the requested function on an object that is not in the filter graph.";
                break;
            case -2147220895:
                result = "Cannot get or set time related information on an object that is using a time format of TIME_FORMAT_NONE.";
                break;
            case -2147220894:
                result = "The connection cannot be made because the stream is read only and the filter alters the data.";
                break;
            case -2147220892:
                result = "The buffer is not full enough.";
                break;
            case -2147220891:
                result = "Cannot play back the file.  The format is not supported.";
                break;
            case -2147220890:
                result = "Pins cannot connect due to not supporting the same transport.";
                break;
            case -2147220887:
                result = "The Video CD can't be read correctly by the device or is the data is corrupt.";
                break;
            case -2147220879:
                result = "There is not enough Video Memory at this display resolution and number of colors. Reducing resolution might help.";
                break;
            case -2147220878:
                result = "The VideoPort connection negotiation process has failed.";
                break;
            case -2147220877:
                result = "Either DirectDraw has not been installed or the Video Card capabilities are not suitable. Make sure t"
                    "he display is not in 16 color mode.";
                break;
            case -2147220876:
                result = "No VideoPort hardware is available, or the hardware is not responding.";
                break;
            case -2147220875:
                result = "No Capture hardware is available, or the hardware is not responding.";
                break;
            case -2147220874:
                result = "This User Operation is inhibited by DVD Content at this time.";
                break;
            case -2147220873:
                result = "This Operation is not permitted in the current domain.";
                break;
            case -2147220872:
                result = "The specified button is invalid or is not present at the current time, or there is no button present "
                    "at the specified location.";
                break;
            case -2147220871:
                result = "DVD-Video playback graph has not been built yet.";
                break;
            case -2147220870:
                result = "DVD-Video playback graph building failed.";
                break;
            case -2147220869:
                result = "DVD-Video playback graph could not be built due to insufficient decoders.";
                break;
            case -2147220868:
                result = "Version number of DirectDraw not suitable. Make sure to install dx5 or higher version.";
                break;
            case -2147220867:
                result = "Copy protection cannot be enabled. Please make sure any other copy protected content is not being shown now.";
                break;
            case -2147220865:
                result = "This object cannot be used anymore as its time has expired.";
                break;
            case -2147220863:
                result = "The operation cannot be performed at the current playback speed.";
                break;
            case -2147220862:
                result = "The specified menu doesn't exist.";
                break;
            case -2147220861:
                result = "The specified command was either cancelled or no longer exists.";
                break;
            case -2147220860:
                result = "The data did not contain a recognized version.";
                break;
            case -2147220859:
                result = "The state data was corrupt.";
                break;
            case -2147220858:
                result = "The state data is from a different disc.";
                break;
            case -2147220857:
                result = "The region was not compatible with the current drive.";
                break;
            case -2147220856:
                result = "The requested DVD stream attribute does not exist.";
                break;
            case -2147220855:
                result = "Currently there is no GoUp (Annex J user function) program chain (PGC).";
                break;
            case -2147220854:
                result = "The current parental level was too low.";
                break;
            case -2147220853:
                result = "The current audio is not karaoke content.";
                break;
            case -2147220850:
                result = "Frame step is not supported on this configuration.";
                break;
            case -2147220849:
                result = "The specified stream is disabled and cannot be selected.";
                break;
            case -2147220848:
                result = "The operation depends on the current title number, however the navigator has not yet entered the VTSM"
                    " or the title domains, so the 'current' title index is unknown.";
                break;
            case -2147220847:
                result = "The specified path does not point to a valid DVD disc.";
                break;
            case -2147220846:
                result = "There is currently no resume information.";
                break;
            case -2147220845:
                result = "This thread has already blocked this output pin.  There is no need to call IPinFlowControl::Block() again.";
                break;
            case -2147220844:
                result = "IPinFlowControl::Block() has been called on another thread.  The current thread cannot make any assum"
                    "ptions about this pin's block state.";
                break;
            case -2147220843:
                result = "An operation failed due to a certification failure.";
                break;
            default:
                return "n/a";
            }
            return result;
        }
        if (a1 <= -2147024894)
        {
            if (a1 == -2147024894)
                return "The system cannot find the file specified.";
            if (a1 > -2147220478)
            {
                switch (a1)
                {
                case -2147220477:
                    result = "No stream can be found with the specified attributes.";
                    break;
                case -2147220476:
                    result = "Seeking not supported for this object.";
                    break;
                case -2147220475:
                    result = "The stream formats are not compatible.";
                    break;
                case -2147220474:
                    result = "The sample is busy.";
                    break;
                case -2147220473:
                    result = "The object can't accept the call because its initialize function or equivalent has not been called.";
                    break;
                case -2147220472:
                    result = "MS_E_SOURCEALREADYDEFINED";
                    break;
                case -2147220471:
                    result = "The stream type is not valid for this operation.";
                    break;
                case -2147220470:
                    result = "The object is not in running state.";
                    break;
                default:
                    return "n/a";
                }
            }
            else
            {
                if (a1 == -2147220478)
                    return "The INF file for the selected device could not be found or is invalid or is damaged. & The specified pu"
                    "rpose ID can't be used for the call.";
                if (a1 > -2147220731)
                {
                    switch (a1)
                    {
                    case -2147220494:
                        return "A registry entry is corrupt.";
                    case -2147220481:
                        return "Device installer errors.";
                    case -2147220480:
                        return "Registry entry or DLL for class installer invalid or class installer not found.";
                    case -2147220479:
                        return "The user cancelled the install operation. & The stream already has allocated samples and the surfac"
                            "e doesn't match the sample format.";
                    }
                    return "n/a";
                }
                switch (a1)
                {
                case -2147220731:
                    return "DIERR_DRIVERFIRST+5";
                case -2147220735:
                    return "DIERR_DRIVERFIRST+1";
                case -2147220734:
                    return "DIERR_DRIVERFIRST+2";
                case -2147220733:
                    return "DIERR_DRIVERFIRST+3";
                default:
                    return "DIERR_DRIVERFIRST+4";
                }
            }
            return result;
        }
        if (a1 > -2147024875)
        {
            switch (a1)
            {
            case -2147024866:
                return "Access to the device has been lost.  It must be re-acquired.";
            case -2147024809:
                return "An invalid parameter was passed to the returning function";
            case -2147024777:
                return "The object could not be created due to an incompatible driver version or mismatched or incomplete driver components.";
            case -2147024726:
                return "The operation cannot be performed while the device is acquired.";
            case -2147024637:
                return "No more items.";
            case -2147023746:
                return "The application requires a newer version of DirectInput.";
            }
            return "n/a";
        }
        if (a1 != -2147024875)
        {
            switch (a1)
            {
            case -2147024893:
                goto LABEL_928;
            case -2147024892:
                goto LABEL_929;
            case -2147024891:
                result = "Access is denied";
                break;
            case -2147024890:
                return "Invalid handle";
            case -2147024888:
                goto LABEL_932;
            case -2147024887:
                goto LABEL_933;
            case -2147024886:
                goto LABEL_934;
            case -2147024885:
                goto LABEL_935;
            case -2147024884:
                result = "The operation cannot be performed unless the device is acquired.";
                break;
            case -2147024882:
                result = "Ran out of memory";
                break;
            default:
                return "n/a";
            }
            return result;
        }
        return "This object has not been initialized";
    }
    if (a1 == -2147220903)
        return "Cannot play back the video stream: format 'RPZA' is not supported.";
    if (a1 > -2147220962)
    {
        switch (a1)
        {
        case -2147220961:
            result = "No matching color key is available.";
            break;
        case -2147220960:
            result = "No palette is available.";
            break;
        case -2147220959:
            result = "Display does not use a palette.";
            break;
        case -2147220958:
            result = "Too many colors for the current display settings.";
            break;
        case -2147220957:
            result = "The state changed while waiting to process the sample.";
            break;
        case -2147220956:
            result = "The operation could not be performed because the filter is not stopped.";
            break;
        case -2147220955:
            result = "The operation could not be performed because the filter is not paused.";
            break;
        case -2147220954:
            result = "The operation could not be performed because the filter is not running.";
            break;
        case -2147220953:
            result = "The operation could not be performed because the filter is in the wrong state.";
            break;
        case -2147220952:
            result = "The sample start time is after the sample end time.";
            break;
        case -2147220951:
            result = "The supplied rectangle is invalid.";
            break;
        case -2147220950:
            result = "This pin cannot use the supplied media type.";
            break;
        case -2147220949:
            result = "This sample cannot be rendered.";
            break;
        case -2147220948:
            result = "This sample cannot be rendered because the end of the stream has been reached.";
            break;
        case -2147220947:
            result = "An attempt to add a filter with a duplicate name failed.";
            break;
        case -2147220946:
            result = "A time-out has expired.";
            break;
        case -2147220945:
            result = "The file format is invalid.";
            break;
        case -2147220944:
            result = "The list has already been exhausted.";
            break;
        case -2147220943:
            result = "The filter graph is circular.";
            break;
        case -2147220942:
            result = "Updates are not allowed in this state.";
            break;
        case -2147220941:
            result = "An attempt was made to queue a command for a time in the past.";
            break;
        case -2147220940:
            result = "The queued command has already been canceled.";
            break;
        case -2147220939:
            result = "Cannot render the file because it is corrupt.";
            break;
        case -2147220938:
            result = "An overlay advise link already exists.";
            break;
        case -2147220936:
            result = "No full-screen modes are available.";
            break;
        case -2147220935:
            result = "This Advise cannot be canceled because it was not successfully set.";
            break;
        case -2147220934:
            result = "A full-screen mode is not available.";
            break;
        case -2147220933:
            result = "Cannot call IVideoWindow methods while in full-screen mode.";
            break;
        case -2147220928:
            result = "The media type of this file is not recognized.";
            break;
        case -2147220927:
            result = "The source filter for this file could not be loaded.";
            break;
        case -2147220925:
            result = "A file appeared to be incomplete.";
            break;
        case -2147220924:
            result = "The version number of the file is invalid.";
            break;
        case -2147220921:
            result = "This file is corrupt: it contains an invalid class identifier.";
            break;
        case -2147220920:
            result = "This file is corrupt: it contains an invalid media type.";
            break;
        case -2147220919:
            result = "No time stamp has been set for this sample.";
            break;
        case -2147220911:
            result = "No media time stamp has been set for this sample.";
            break;
        case -2147220910:
            result = "No media time format has been selected.";
            break;
        case -2147220909:
            result = "Cannot change balance because audio device is mono only.";
            break;
        case -2147220907:
            return "Cannot play back the video stream: no suitable decompressor could be found.";
        case -2147220906:
            result = "Cannot play back the audio stream: no audio hardware is available, or the hardware is not responding.";
            break;
        default:
            return "n/a";
        }
    }
    else
    {
        if (a1 == -2147220962)
            return "Setting a palette would conflict with the color key already set.";
        if (a1 > -2147220983)
        {
            switch (a1)
            {
            case -2147220982:
                result = "SendDeviceData failed because more information was requested to be sent than can be sent to the devic"
                    "e.  Some devices have restrictions on how much data can be sent to them.  (For example, there might b"
                    "e a limit on the number of buttons that can be pressed at once.) & No sample buffer allocator is available.";
                break;
            case -2147220981:
                result = "A mapper file function failed because reading or writing the user or IHV settings file failed. & A ru"
                    "n-time error occurred.";
                break;
            case -2147220980:
                result = "No buffer space has been set";
                break;
            case -2147220979:
                result = "The buffer is not big enough.";
                break;
            case -2147220978:
                result = "An invalid alignment was specified.";
                break;
            case -2147220977:
                result = "Cannot change allocated memory while the filter is active.";
                break;
            case -2147220976:
                result = "One or more buffers are still active.";
                break;
            case -2147220975:
                result = "Cannot allocate a sample when the allocator is not active.";
                break;
            case -2147220974:
                result = "Cannot allocate memory because no size has been set.";
                break;
            case -2147220973:
                result = "Cannot lock for synchronization because no clock has been defined.";
                break;
            case -2147220972:
                result = "Quality messages could not be sent because no quality sink has been defined.";
                break;
            case -2147220971:
                result = "A required interface has not been implemented.";
                break;
            case -2147220970:
                result = "An object or name was not found.";
                break;
            case -2147220969:
                result = "No combination of intermediate filters could be found to make the connection.";
                break;
            case -2147220968:
                result = "No combination of filters could be found to render the stream.";
                break;
            case -2147220967:
                result = "Could not change formats dynamically.";
                break;
            case -2147220966:
                result = "No color key has been set.";
                break;
            case -2147220965:
                result = "Current pin connection is not using the IOverlay transport.";
                break;
            case -2147220964:
                result = "Current pin connection is not using the IMemInputPin transport.";
                break;
            case -2147220963:
                result = "Setting a color key would conflict with the palette already set.";
                break;
            default:
                return "n/a";
            }
        }
        else
        {
            if (a1 == -2147220983)
                return "The operation could not be completed because the device is not plugged in. & The operation cannot be perf"
                "ormed because the pins are not connected.";
            if (a1 <= -2147221007)
            {
                if (a1 == -2147221007)
                    return "CoInitialize has already been called.";
                if (a1 > -2147467259)
                {
                    switch (a1)
                    {
                    case -2147418113:
                        return "Catastrophic failure";
                    case -2147221232:
                        return "This object does not support aggregation";
                    case -2147221164:
                        return "Class not registered";
                    case -2147221008:
                        return "CoInitialize has not been called.";
                    }
                }
                else
                {
                    switch (a1)
                    {
                    case -2147467259:
                        return "An undetermined error occurred";
                    case -2147483638:
                        return "The data necessary to complete this operation is not yet available.";
                    case -2147467263:
                        return "The function called is not supported at this time";
                    case -2147467262:
                        return "The requested COM interface is not available";
                    case -2147467261:
                        return "Invalid pointer";
                    case -2147467260:
                        return "Operation aborted";
                    }
                }
                return "n/a";
            }
            switch (a1)
            {
            case -2147220992:
                result = "Unable to IDirectInputJoyConfig_Acquire because the user does not have sufficient privileges to chang"
                    "e the joystick configuration. & An invalid media type was specified";
                break;
            case -2147220991:
                result = "The device is full. & An invalid media subtype was specified.";
                break;
            case -2147220990:
                result = "Not all the requested information fit into the buffer. & This object can only be created as an aggregated object.";
                break;
            case -2147220989:
                result = "The effect is not downloaded. & The enumerator has become invalid.";
                break;
            case -2147220988:
                result = "The device cannot be reinitialized because there are still effects attached to it. & At least one of "
                    "the pins involved in the operation is already connected.";
                break;
            case -2147220987:
                result = "The operation cannot be performed unless the device is acquired in DISCL_EXCLUSIVE mode. & This opera"
                    "tion cannot be performed because the filter is active.";
                break;
            case -2147220986:
                result = "The effect could not be downloaded because essential information is missing.  For example, no axes ha"
                    "ve been associated with the effect, or no type-specific information has been created. & One of the sp"
                    "ecified pins supports no media types.";
                break;
            case -2147220985:
                result = "Attempted to read buffered device data from a device that is not buffered. & There is no common media"
                    " type between these pins.";
                break;
            case -2147220984:
                result = "An attempt was made to modify parameters of an effect while it is playing.  Not all hardware devices "
                    "support altering the parameters of an effect while it is playing. & Two pins of the same direction ca"
                    "nnot be connected together.";
                break;
            default:
                return "n/a";
            }
        }
    }
    return result;
}

void __cdecl  R_FatalInitError(const char *msg)
{
    Com_Printf(8, "********** DirectX returned an unrecoverable error code during initialization  **********\n");
    Com_Printf(8, "********** Initialization also happens while playing if DirectX loses a device **********\n");
    Com_Printf(8, "********** Consult the readme for how to continue from this problem            **********\n");
    Com_Printf(8, "\n%s\n", msg);
    Sys_DirectXFatalError();
}

void __cdecl  R_FatalLockError(HRESULT hr)
{
    const char *v1; // eax

    Com_Printf(8, "********** DirectX failed a call to lock a vertex buffer or an index buffer **********\n");
    v1 = R_ErrorDescription(hr);
    Com_Printf(8, "********** error information:  %s\n", v1);
    Sys_DirectXFatalError();
}

const char *__cdecl R_ErrorDescription(HRESULT hr)
{
    return DXGetErrorDescription9A(hr);
}

void __cdecl R_SetColorMappings()
{
    GfxGammaRamp gammaRamp; // [esp+0h] [ebp-208h] BYREF

    if (vidConfig.deviceSupportsGamma)
    {
        R_CalcGammaRamp(&gammaRamp);
        RB_SetGammaRamp(&gammaRamp);
    }
}

void __cdecl R_CalcGammaRamp(GfxGammaRamp *gammaRamp)
{
    float v1; // [esp+8h] [ebp-30h]
    float v2; // [esp+Ch] [ebp-2Ch]
    float v3; // [esp+18h] [ebp-20h]
    unsigned __int16 adjustedColorValue; // [esp+28h] [ebp-10h]
    unsigned __int16 colorTableIndex; // [esp+2Ch] [ebp-Ch]
    float exponent; // [esp+30h] [ebp-8h]

    iassert( gammaRamp );
    if (r_gamma->current.value <= 0.0)
        MyAssertHandler(
            ".\\r_init.cpp",
            741,
            0,
            "%s\n\t(r_gamma->current.value) = %g",
            "(r_gamma->current.value > 0)",
            r_gamma->current.value);
    exponent = 1.0 / r_gamma->current.value;
    for (colorTableIndex = 0; colorTableIndex < 0x100u; ++colorTableIndex)
    {
        if (exponent == 1.0)
        {
            adjustedColorValue = 257 * colorTableIndex;
        }
        else
        {
            v2 = (double)colorTableIndex / 255.0;
            v1 = pow(v2, exponent);
            if (v1 < 0.0 || v1 >= 1.000007629394531)
                MyAssertHandler(
                    ".\\r_init.cpp",
                    754,
                    0,
                    "%s\n\t(unitScaleValue) = %g",
                    "(unitScaleValue >= 0 && unitScaleValue < 1 + 0.5f / 65535)",
                    v1);
            adjustedColorValue = (int)(v1 * 65535.0f);
        }
        gammaRamp->entries[colorTableIndex] = adjustedColorValue;
    }
}

void __cdecl R_GammaCorrect(unsigned __int8 *buffer, int bufSize)
{
    int tableIndex; // [esp+0h] [ebp-210h]
    GfxGammaRamp gammaRamp; // [esp+8h] [ebp-208h] BYREF
    int inValue; // [esp+20Ch] [ebp-4h]

    iassert( buffer );
    iassert( (bufSize > 0) );
    R_CalcGammaRamp(&gammaRamp);
    for (tableIndex = 0; tableIndex < bufSize; ++tableIndex)
    {
        inValue = buffer[tableIndex];
        buffer[tableIndex] = 255 * gammaRamp.entries[inValue] / 0xFFFF;
    }
}

void __cdecl SetGfxConfig(const GfxConfiguration *config)
{
    iassert(config);
    bcassert(config->maxClientViews, GFX_MAX_CLIENT_VIEWS);
    iassert(config->critSectCount == CRITSECT_COUNT);

    memcpy(&gfxCfg, config, sizeof(gfxCfg));
}

void __cdecl R_InitThreads()
{
    iassert(!r_glob.isRenderingRemoteUpdate);

    R_InitRenderThread();
    R_InitWorkerThreads();
}

void __cdecl R_ShutdownStreams()
{
    if (dx.device)
    {
        if (!dx.deviceLost)
            R_ClearAllStreamSources(&gfxCmdBufState.prim);
    }
}

void __cdecl R_ShutdownMaterialUsage()
{
    VertUsage *vertUsage; // [esp+0h] [ebp-Ch]
    unsigned __int16 hashIndex; // [esp+4h] [ebp-8h]
    VertUsage *nextVertUsage; // [esp+8h] [ebp-4h]

    for (hashIndex = 0; hashIndex < ARRAY_COUNT(rg.materialUsage); ++hashIndex)
    {
        for (vertUsage = rg.materialUsage[hashIndex].verts; vertUsage; vertUsage = nextVertUsage)
        {
            nextVertUsage = vertUsage->next;
            Z_Free((char *)vertUsage, 0);
        }
        rg.materialUsage[hashIndex].verts = 0;
    }
}

void __cdecl R_Shutdown(int destroyWindow)
{
    if (rg.registered)
    {
        R_SyncRenderThread();
        rg.registered = 0;
        iassert(r_glob.haveThreadOwnership);
        r_glob.startedRenderThread = 0;
        R_ShutdownStreams();
        R_ShutdownMaterialUsage();
        R_ShutdownDebug();
        R_SaveLightVisHistory();
        R_ShutdownLightDefs();
        R_ShutdownWorld();
        if (!IsFastFileLoad())
        {
            R_ShutdownLoadWater();
            R_ShutdownFonts();
            Material_Shutdown();
            R_ShutdownImages();
        }
        R_ResetModelLighting();
        rgp.world = 0;
        R_UnlockSkinnedCache();
        R_FlushStaticModelCache();
        if (destroyWindow)
        {
            R_ShutdownDirect3D();
            R_ShutdownRenderCommands();
        }
        R_UnregisterCmds();
    }
}

void R_UnloadGraphicsAssets()
{
    DB_ShutdownXAssets();
}

void R_ShutdownDirect3D()
{
    IDirect3DSurface9 *var; // [esp+4h] [ebp-8h]
    IDirect3DDevice9 *varCopy; // [esp+8h] [ebp-4h]

    if (IsFastFileLoad())
        R_UnloadGraphicsAssets();
    R_Cinematic_Shutdown();
    R_ReleaseForShutdownOrReset();
    while (dx.windowCount)
    {
        if (!dx.windows[--dx.windowCount].hwnd)
            MyAssertHandler(".\\r_init.cpp", 2205, 0, "%s", "dx.windows[dx.windowCount].hwnd");
        if (IsWindow(dx.windows[dx.windowCount].hwnd))
            DestroyWindow(dx.windows[dx.windowCount].hwnd);
        dx.windows[dx.windowCount].hwnd = 0;
    }
    if (dx.device)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.device->Release()\n");
            varCopy = dx.device;
            dx.device = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>((IDirect3DSurface9 *)varCopy, "dx.device", ".\\r_init.cpp", 2214);
        } while (alwaysfails);
    }
    if (dx.d3d9)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.d3d9->Release()\n");
            var = (IDirect3DSurface9 *)dx.d3d9;
            dx.d3d9 = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(var, "dx.d3d9", ".\\r_init.cpp", 2217);
        } while (alwaysfails);
    }
}

void __cdecl R_UnloadWorld()
{
    iassert( IsFastFileLoad() );
    if (rgp.world)
        Sys_Error("Cannot unload bsp while it is in use");
}

void __cdecl R_BeginRegistration(vidConfig_t *vidConfigOut)
{
    iassert(!rg.registered);
    R_Init();
    iassert(dx.d3d9 && dx.device);
    iassert(rg.registered == true);
    iassert(vidConfigOut);
    memcpy(vidConfigOut, &vidConfig, sizeof(vidConfig_t));
    iassert(!r_glob.startedRenderThread);
    r_glob.startedRenderThread = 1;
    R_ReleaseThreadOwnership();
}

void R_Init()
{
    HRESULT hr; // [esp+0h] [ebp-4h]

    Com_Printf(8, "----- R_Init -----\n");
    Swap_Init();
    R_Register();
    R_InitGlobalStructs();
    R_InitDrawMethod();
    R_InitGraphicsApi();
    RB_RegisterBackendAssets();
    R_InitWater();
    if (!dx.deviceLost)
    {
        hr = dx.device->TestCooperativeLevel();
        if (hr != -2005530520 && hr != -2005530519)
        {
            dx.sunSpriteSamples = RB_CalcSunSpriteSamples();
            if (!dx.sunSpriteSamples)
            {
                Com_Printf(8, "Sun sprite occlusion query calibration failed; reverting to low-quality sun visibility test");
                RB_FreeSunSpriteQueries();
            }
        }
    }
    RB_ProfileInit();
}

char __cdecl R_ReduceWindowSettings()
{
    if (r_aaSamples->current.integer <= 1)
    {
        if (r_displayRefresh->current.integer <= 0 || vidConfig.displayFrequency <= 0x3C)
        {
            if (r_mode->current.integer <= 0 || vidConfig.displayWidth <= 0x280 && vidConfig.displayHeight <= 0x1E0)
            {
                return 0;
            }
            else
            {
                Dvar_SetInt((dvar_s *)r_mode, r_mode->current.integer - 1);
                return 1;
            }
        }
        else
        {
            Dvar_SetInt((dvar_s *)r_displayRefresh, r_displayRefresh->current.integer - 1);
            return 1;
        }
    }
    else
    {
        Dvar_SetInt((dvar_s *)r_aaSamples, r_aaSamples->current.integer - 1);
        return 1;
    }
}

void R_InitGraphicsApi()
{
    GfxWindowParms wndParms; // [esp+4h] [ebp-28h] BYREF

    iassert( (dx.device != NULL) == (dx.d3d9 != NULL) );
    if (dx.device)
    {
        R_InitSystems();
    }
    else
    {
        R_PreCreateWindow();
        while (1)
        {
            R_SetWndParms(&wndParms);
            if (R_CreateGameWindow(&wndParms))
                break;
            if (!R_ReduceWindowSettings())
                R_FatalInitError("Couldn't initialize renderer");
        }
    }
}

void R_InitSystems()
{
    R_InitImages();
    Material_Init();
    R_InitFonts();
    R_InitLoadWater();
    R_InitLightDefs();
    R_ClearFogs();
    R_InitDebug();
    rg.registered = 1;
}

int __cdecl R_CompareRefreshRates(_DWORD *e0, _DWORD *e1)
{
    return *e0 - *e1;
}

int __cdecl R_AddValidResolution(int width, int height, int resolutionCount, int (*availableResolutions)[2])
{
    iassert( (resolutionCount >= 0) );
    if (resolutionCount > 0
        && (*availableResolutions)[2 * resolutionCount - 2] == width
        && (*availableResolutions)[2 * resolutionCount - 1] == height)
    {
        return resolutionCount;
    }
    if (width < 640 || height < 480)
        return resolutionCount;
    (*availableResolutions)[2 * resolutionCount] = width;
    (*availableResolutions)[2 * resolutionCount + 1] = height;
    return resolutionCount + 1;
}

int __cdecl R_CompareDisplayModes(_DWORD *e0, _DWORD *e1)
{
    int delta; // [esp+0h] [ebp-Ch]

    delta = *e0 - *e1;
    if (*e0 == *e1)
    {
        delta = e0[1] - e1[1];
        if (!delta)
            return e0[2] - e1[2];
    }
    return delta;
}

int __cdecl R_AddValidRefreshRate(int refreshRate, int rateCount, int *availableRefreshRates)
{
    int rateIndex; // [esp+4h] [ebp-4h]
    int rateIndexa; // [esp+4h] [ebp-4h]

    for (rateIndex = 0; rateIndex < rateCount; ++rateIndex)
    {
        if (availableRefreshRates[rateIndex] == refreshRate)
            return rateCount;
    }
    for (rateIndexa = rateCount; rateIndexa > 0 && availableRefreshRates[rateIndexa - 1] > refreshRate; --rateIndexa)
        availableRefreshRates[rateIndexa] = availableRefreshRates[rateIndexa - 1];
    availableRefreshRates[rateIndexa] = refreshRate;
    return rateCount + 1;
}

void __cdecl R_EnumDisplayModes(unsigned int adapterIndex)
{
    const char *v1; // eax
    int v2; // eax
    int v3; // eax
    int refreshRateCount; // [esp+0h] [ebp-C30h]
    int resolutionCount; // [esp+4h] [ebp-C2Ch]
    int availableResolutions[256][2]; // [esp+8h] [ebp-C28h] BYREF
    HRESULT hr; // [esp+80Ch] [ebp-424h]
    unsigned int modeCountReported; // [esp+810h] [ebp-420h]
    unsigned int modeIndex; // [esp+814h] [ebp-41Ch]
    int resolutionIndex; // [esp+818h] [ebp-418h]
    int defaultRefreshRateIndex; // [esp+81Ch] [ebp-414h]
    int availableRefreshRates[257]; // [esp+820h] [ebp-410h] BYREF
    char *modeText; // [esp+C24h] [ebp-Ch]
    int refreshRateIndex; // [esp+C28h] [ebp-8h]
    int defaultResolutionIndex; // [esp+C2Ch] [ebp-4h]

    modeCountReported = dx.d3d9->GetAdapterModeCount(adapterIndex, D3DFMT_X8R8G8B8);
    dx.displayModeCount = 0;
    for (modeIndex = 0; modeIndex < modeCountReported && dx.displayModeCount < 0x100; ++modeIndex)
    {
        hr = dx.d3d9->EnumAdapterModes(
            adapterIndex,
            D3DFMT_X8R8G8B8,
            modeIndex,
            &dx.displayModes[dx.displayModeCount]);
        if (hr >= 0)
        {
            if (!dx.resolutionNameTable[4 * dx.displayModeCount - 1022])
                dx.resolutionNameTable[4 * dx.displayModeCount - 1022] = (const char *)60;
            ++dx.displayModeCount;
        }
    }
    qsort(dx.displayModes, dx.displayModeCount, 0x10u, (int(__cdecl *)(const void *, const void *))R_CompareDisplayModes);
    resolutionCount = 0;
    refreshRateCount = 0;
    for (modeIndex = 0; modeIndex < dx.displayModeCount; ++modeIndex)
    {
        resolutionCount = R_AddValidResolution(
            dx.displayModes[modeIndex].Width,
            (int)dx.resolutionNameTable[4 * modeIndex - 1023],
            resolutionCount,
            availableResolutions);
        refreshRateCount = R_AddValidRefreshRate(
            (int)dx.resolutionNameTable[4 * modeIndex - 1022],
            refreshRateCount,
            availableRefreshRates);
    }
    modeText = dx.modeText;
    if (!resolutionCount)
    {
        v1 = va("No valid resolutions of %i x %i or above found", 640, 480);
        R_FatalInitError(v1);
    }
    defaultResolutionIndex = 0;
    for (resolutionIndex = 0; resolutionIndex < resolutionCount; ++resolutionIndex)
    {
        dx.resolutionNameTable[resolutionIndex] = modeText;
        v2 = sprintf(modeText, "%ix%i", availableResolutions[resolutionIndex][0], availableResolutions[resolutionIndex][1]);
        modeText += v2 + 1;
        if (availableResolutions[defaultResolutionIndex][0] < 640 || availableResolutions[defaultResolutionIndex][1] < 480)
            defaultResolutionIndex = resolutionIndex;
    }
    dx.resolutionNameTable[resolutionIndex] = 0;
    r_mode = Dvar_RegisterEnum(
        "r_mode",
        dx.resolutionNameTable,
        defaultResolutionIndex,
        DVAR_ARCHIVE | DVAR_LATCH,
        "Direct X resolution mode");
    qsort(availableRefreshRates, refreshRateCount, 4u, (int(__cdecl *)(const void *, const void *))R_CompareRefreshRates);
    defaultRefreshRateIndex = 0;
    for (refreshRateIndex = 0; refreshRateIndex < refreshRateCount; ++refreshRateIndex)
    {
        dx.refreshRateNameTable[refreshRateIndex] = modeText;
        v3 = sprintf(modeText, "%i Hz", availableRefreshRates[refreshRateIndex]);
        modeText += v3 + 1;
        if (availableRefreshRates[defaultRefreshRateIndex] < 60)
            defaultRefreshRateIndex = refreshRateIndex;
    }
    dx.refreshRateNameTable[refreshRateIndex] = 0;
    r_displayRefresh = Dvar_RegisterEnum(
        "r_displayRefresh",
        dx.refreshRateNameTable,
        defaultRefreshRateIndex,
        DVAR_ARCHIVE | DVAR_LATCH | DVAR_AUTOEXEC,
        "Refresh rate");

    r_noborder = Dvar_RegisterBool("r_noborder", false, DVAR_ARCHIVE, "Do not use a border in windowed mode");
}

char __cdecl R_PreCreateWindow()
{
    if (dx.d3d9)
    {
        if (!alwaysfails)
            MyAssertHandler(".\\r_init.cpp", 1345, 1, "D3D re-initialized before being shutdown");
    }
    else
    {
        Com_Printf(8, "Getting Direct3D 9 interface...\n");
        dx.d3d9 = Direct3DCreate9(0x20u);
        if (!dx.d3d9)
        {
            Com_Printf(8, "Direct3D 9 failed to initialize\n");
            return 0;
        }
    }
    dx.adapterIndex = R_ChooseAdapter();
    R_StoreDirect3DCaps(dx.adapterIndex);
    R_EnumDisplayModes(dx.adapterIndex);
    return 1;
}

void __cdecl R_RespondToMissingCaps(DxCapsResponse response, const char *msg, int *allowedPaths)
{
    const char *v3; // eax
    void (*Printf)(int, const char *, ...); // [esp+4h] [ebp-4h]

    if (response == DX_CAPS_RESPONSE_WARN)
    {
        Printf = (void (*)(int, const char *, ...))Com_PrintWarning;
        Com_PrintWarning(8, "Video card or driver %s.\n", msg);
    }
    else
    {
        Printf = (void (*)(int, const char *, ...))Com_Printf;
        Com_Printf(8, "Video card or driver %s.\n", msg);
    }
    switch (response)
    {
    case DX_CAPS_RESPONSE_QUIT:
        Com_Error(ERR_FATAL, "Video card or driver %s.\n", msg);
        break;
    case DX_CAPS_RESPONSE_WARN:
    case DX_CAPS_RESPONSE_INFO:
        return;
    case DX_CAPS_RESPONSE_FORBID_SM3:
        *allowedPaths &= ~2u;
        Printf(8, "  Shader model 3.0 rendering path will not be available.\n");
        break;
    default:
        if (!alwaysfails)
        {
            v3 = va("unhandled response %i", response);
            MyAssertHandler(".\\r_caps.cpp", 159, 1, v3);
        }
        break;
    }
}

int __cdecl R_CheckDxCaps(const _D3DCAPS9 *caps)
{
    unsigned int integer; // [esp+0h] [ebp-10h]
    unsigned int checkIndex; // [esp+4h] [ebp-Ch]
    unsigned int checkIndexa; // [esp+4h] [ebp-Ch]
    unsigned int bits; // [esp+8h] [ebp-8h]
    int allowedPaths; // [esp+Ch] [ebp-4h] BYREF

    allowedPaths = 3;
    for (checkIndex = 0; checkIndex < 0x21; ++checkIndex)
    {
        bits = *(_D3DDEVTYPE *)((char *)&caps->DeviceType + s_capsCheckBits[checkIndex].offset);
        if ((!s_capsCheckBits[checkIndex].clearBits || (s_capsCheckBits[checkIndex].clearBits & ~bits) != 0)
            && (!s_capsCheckBits[checkIndex].setBits || (s_capsCheckBits[checkIndex].setBits & bits) != 0))
        {
            R_RespondToMissingCaps(s_capsCheckBits[checkIndex].response, s_capsCheckBits[checkIndex].msg, &allowedPaths);
        }
    }
    for (checkIndexa = 0; checkIndexa < 0xA; ++checkIndexa)
    {
        integer = *(_D3DDEVTYPE *)((char *)&caps->DeviceType + s_capsCheckInt[checkIndexa].offset);
        if (integer < s_capsCheckInt[checkIndexa].min || integer > s_capsCheckInt[checkIndexa].max)
            R_RespondToMissingCaps(s_capsCheckInt[checkIndexa].response, s_capsCheckInt[checkIndexa].msg, &allowedPaths);
    }
    return allowedPaths;
}

const char *__cdecl R_DescribeRenderer(int renderer)
{
    if (renderer == 1)
        return "Shader model 3.0";
    iassert( (renderer == GFX_RENDERER_SHADER_2) );
    return "Shader model 2.0";
}

void __cdecl R_PickRenderer(_D3DCAPS9 *caps)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // [esp-4h] [ebp-10h]
    const char *v6; // [esp-4h] [ebp-10h]
    GfxRenderer rendererChosen; // [esp+0h] [ebp-Ch]
    GfxRenderer rendererChosena; // [esp+0h] [ebp-Ch]
    int allowedPaths; // [esp+4h] [ebp-8h]
    int rendererIter; // [esp+8h] [ebp-4h]

    Com_Printf(
        8,
        "Pixel shader version is %i.%i\n",
        BYTE1(caps->PixelShaderVersion),
        (unsigned __int8)caps->PixelShaderVersion);
    Com_Printf(
        8,
        "Vertex shader version is %i.%i\n",
        BYTE1(caps->VertexShaderVersion),
        (unsigned __int8)caps->VertexShaderVersion);
    allowedPaths = R_CheckDxCaps(caps);
    rendererChosen = GFX_RENDERER_COUNT;
    for (rendererIter = 0; rendererIter != 2; ++rendererIter)
    {
        if ((allowedPaths & (1 << rendererIter)) != 0)
        {
            rendererChosen = (GfxRenderer)rendererIter;
            v1 = R_DescribeRenderer(rendererIter);
            Com_Printf(8, "%s code path is available.\n", v1);
        }
    }
    if (rendererChosen == GFX_RENDERER_COUNT)
        Com_Error(ERR_FATAL, "No valid rendering code path detected.\n");
    if (r_rendererPreference->current.integer == 2)
    {
        v4 = R_DescribeRenderer(rendererChosen);
        Com_Printf(8, "Using %s code path because it is the best available path on this hardware.\n", v4);
    }
    else
    {
        if ((allowedPaths & (1 << r_rendererPreference->current.integer)) != 0)
        {
            rendererChosena = (GfxRenderer)r_rendererPreference->current.integer;
            v5 = Dvar_EnumToString(r_rendererPreference);
            v2 = R_DescribeRenderer(rendererChosena);
            Com_Printf(8, "Using %s code path because r_rendererPreference is set to %s.\n", v2, v5);
            Dvar_SetInt((dvar_s *)r_rendererInUse, rendererChosena);
            return;
        }
        v6 = R_DescribeRenderer(r_rendererPreference->current.integer);
        v3 = R_DescribeRenderer(rendererChosen);
        Com_Printf(8, "Using %s code path because the requested %s code path is unavailable.\n", v3, v6);
    }
    Dvar_SetInt((dvar_s *)r_rendererInUse, rendererChosen);
}

bool __cdecl R_CheckTransparencyMsaa(unsigned int adapterIndex)
{
    return r_aaSamples->current.integer != 1
        && dx.d3d9->CheckDeviceFormat(
            adapterIndex,
            D3DDEVTYPE_HAL,
            D3DFMT_X8R8G8B8,
            0,
            D3DRTYPE_SURFACE,
            (_D3DFORMAT)1094800211) == 0;
}

void __cdecl R_StoreDirect3DCaps(unsigned int adapterIndex)
{
    unsigned int MaxUserClipPlanes; // [esp+0h] [ebp-138h]
    unsigned int MaxTextureHeight; // [esp+4h] [ebp-134h]
    _D3DCAPS9 caps; // [esp+8h] [ebp-130h] BYREF

    R_GetDirect3DCaps(adapterIndex, &caps);
    R_PickRenderer(&caps);
    if ((int)caps.MaxTextureHeight < (int)caps.MaxTextureWidth)
        MaxTextureHeight = caps.MaxTextureHeight;
    else
        MaxTextureHeight = caps.MaxTextureWidth;
    vidConfig.maxTextureSize = MaxTextureHeight;
    vidConfig.maxTextureMaps = caps.MaxSimultaneousTextures;
    vidConfig.deviceSupportsGamma = (caps.Caps2 & 0x20000) != 0;
    if (caps.MaxSimultaneousTextures > 0x10)
        MyAssertHandler(
            ".\\r_init.cpp",
            700,
            1,
            "%s\n\t(vidConfig.maxTextureMaps) = %i",
            "(vidConfig.maxTextureMaps <= 16)",
            vidConfig.maxTextureMaps);
    if ((int)caps.MaxUserClipPlanes < 6)
        MaxUserClipPlanes = caps.MaxUserClipPlanes;
    else
        MaxUserClipPlanes = 6;
    gfxMetrics.maxClipPlanes = MaxUserClipPlanes;
    gfxMetrics.hasAnisotropicMinFilter = (caps.TextureFilterCaps & 0x400) != 0;
    gfxMetrics.hasAnisotropicMagFilter = (caps.TextureFilterCaps & 0x4000000) != 0;
    gfxMetrics.maxAnisotropy = caps.MaxAnisotropy;
    gfxMetrics.slopeScaleDepthBias = (caps.RasterCaps & 0x2000000) != 0;
    gfxMetrics.canMipCubemaps = (caps.TextureCaps & 0x10000) != 0;
    gfxMetrics.hasTransparencyMsaa = R_CheckTransparencyMsaa(adapterIndex);
    R_SetShadowmapFormats_DX(adapterIndex);
}

void __cdecl R_GetDirect3DCaps(unsigned int adapterIndex, _D3DCAPS9 *caps)
{
    const char *v2; // eax
    const char *v3; // eax
    int hr; // [esp+0h] [ebp-8h]
    int attempt; // [esp+4h] [ebp-4h]

    iassert( dx.d3d9 );
    attempt = 0;
    while (1)
    {
        //hr = ((int(__thiscall *)(IDirect3D9 *, IDirect3D9 *, unsigned int, int, _D3DCAPS9 *))dx.d3d9->GetDeviceCaps)(
        //    dx.d3d9,
        //    dx.d3d9,
        //    adapterIndex,
        //    1,
        //    caps);
        hr = dx.d3d9->GetDeviceCaps(adapterIndex, D3DDEVTYPE_HAL, caps);
        if (hr >= 0)
            break;
        Sleep(0x64u);
        if (++attempt == 20)
        {
            v2 = R_ErrorDescription(hr);
            v3 = va("GetDeviceCaps failed: %s", v2);
            R_FatalInitError(v3);
        }
    }
}

void __cdecl R_SetShadowmapFormats_DX(unsigned int adapterIndex)
{
    _D3DFORMAT colorFormat; // [esp+0h] [ebp-24h]
    unsigned int formatIndex; // [esp+4h] [ebp-20h]
    _D3DFORMAT depthFormat; // [esp+8h] [ebp-1Ch]

    D3DFORMAT formats[3][2] = {
        { D3DFMT_D24S8, D3DFMT_R5G6B5 },
        { D3DFMT_D24S8, D3DFMT_X8R8G8B8 },
        { D3DFMT_D24S8, D3DFMT_A8R8G8B8   },
    };

    for (formatIndex = 0; formatIndex < 3; ++formatIndex)
    {
        depthFormat = formats[formatIndex][0];
        colorFormat = formats[formatIndex][1];
        bool cond1 = dx.d3d9->CheckDepthStencilMatch(adapterIndex, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, colorFormat, depthFormat);
        bool cond2 = dx.d3d9->CheckDeviceFormat(adapterIndex, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 2, D3DRTYPE_TEXTURE, depthFormat);
        //if (!((int(__thiscall *)(IDirect3D9 *, IDirect3D9 *, unsigned int, int, int, _D3DFORMAT, _D3DFORMAT))dx.d3d9->CheckDepthStencilMatch)(
        //    dx.d3d9,
        //    dx.d3d9,
        //    adapterIndex,
        //    1,
        //    22,
        //    colorFormat,
        //    depthFormat)
        //    && !((int(__thiscall *)(IDirect3D9 *, IDirect3D9 *, unsigned int, int, int, int, int, _D3DFORMAT))dx.d3d9->CheckDeviceFormat)(
        //        dx.d3d9,
        //        dx.d3d9,
        //        adapterIndex,
        //        1,
        //        22,
        //        2,
        //        3,
        //        depthFormat))
        if (cond1 == D3D_OK && cond2 == D3D_OK)
        {
            gfxMetrics.shadowmapFormatPrimary = depthFormat;
            gfxMetrics.shadowmapFormatSecondary = colorFormat;
            gfxMetrics.shadowmapBuildTechType = TECHNIQUE_BUILD_SHADOWMAP_DEPTH;
            gfxMetrics.hasHardwareShadowmap = 1;
            //gfxMetrics.shadowmapSamplerState = 98;
            gfxMetrics.shadowmapSamplerState = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
            return;
        }
    }
    gfxMetrics.shadowmapFormatPrimary = D3DFMT_R32F;
    gfxMetrics.shadowmapFormatSecondary = D3DFMT_D24X8;
    gfxMetrics.shadowmapBuildTechType = TECHNIQUE_BUILD_SHADOWMAP_COLOR;
    gfxMetrics.hasHardwareShadowmap = 0;
    //gfxMetrics.shadowmapSamplerState = 97;
    gfxMetrics.shadowmapSamplerState = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_NEAREST);
}

struct GfxEnumMonitors // sizeof=0x8
{                                       // ...
    int monitorIndex;                   // ...
    HMONITOR__ *foundMonitor;           // ...
};

int __stdcall R_MonitorEnumCallback(HMONITOR__ *monitorHandle, HDC__ *hdc, tagRECT *rect, _DWORD *userData)
{
    if (--*userData)
    {
        return 1;
    }
    else
    {
        userData[1] = (DWORD)monitorHandle;
        return 0;
    }
}

HMONITOR__ *__cdecl R_ChooseMonitor()
{
    POINT pt; // [esp+0h] [ebp-10h]
    GfxEnumMonitors enumData; // [esp+8h] [ebp-8h] BYREF

    if (Dvar_GetBool("r_fullscreen"))
    {
        enumData.monitorIndex = r_monitor->current.integer;
        enumData.foundMonitor = 0;
        EnumDisplayMonitors(0, 0, (MONITORENUMPROC)R_MonitorEnumCallback, (LPARAM)&enumData);
        if (enumData.foundMonitor)
            return enumData.foundMonitor;
    }
    pt.x = Dvar_GetInt("vid_xpos");
    pt.y = Dvar_GetInt("vid_ypos");
    return MonitorFromPoint(pt, 1u);
}

unsigned int __cdecl R_ChooseAdapter()
{
    unsigned int foundAdapterIndex; // [esp+14h] [ebp-470h]
    unsigned int adapterIndex; // [esp+1Ch] [ebp-468h]
    HMONITOR__ *desiredMonitor; // [esp+20h] [ebp-464h]
    unsigned int adapterCount; // [esp+24h] [ebp-460h]
    _D3DADAPTER_IDENTIFIER9 id; // [esp+2Ch] [ebp-458h] BYREF

    desiredMonitor = R_ChooseMonitor();
    foundAdapterIndex = 0;
    adapterCount = dx.d3d9->GetAdapterCount();
    for (adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex)
    {
        if (desiredMonitor)
        {
            if (dx.d3d9->GetAdapterMonitor(adapterIndex) != desiredMonitor)
                continue;
            foundAdapterIndex = adapterIndex;
        }
        if (dx.d3d9->GetAdapterIdentifier(adapterIndex, 0, &id) >= 0
            && !strcmp(id.Description, "NVIDIA NVPerfHUD"))
        {
            return adapterIndex;
        }
    }
    return foundAdapterIndex;
}

// #TODO: need header or some shit to shove all the includes in
#define KISAK_EXTENDED 1

char __cdecl R_CreateWindow(GfxWindowParms *wndParms)
{
    DWORD exStyle; // [esp+0h] [ebp-1Ch]
    DWORD style; // [esp+4h] [ebp-18h]
    HINSTANCE__ *hinst; // [esp+8h] [ebp-14h]
    tagRECT rc; // [esp+Ch] [ebp-10h] BYREF

    iassert( wndParms );
    iassert( wndParms->hwnd == NULL );
    if (wndParms->fullscreen)
    {
        Com_Printf(
            8,
            "Attempting %i x %i fullscreen with 32 bpp at %i hz\n",
            wndParms->displayWidth,
            wndParms->displayHeight,
            wndParms->hz);
        exStyle = WS_EX_TOPMOST;
        style = WS_POPUP;
    }
    else
    {
        Com_Printf(
            8,
            "Attempting %i x %i window at (%i, %i)\n",
            wndParms->displayWidth,
            wndParms->displayHeight,
            wndParms->x,
            wndParms->y);
        exStyle = 0;
#if KISAK_EXTENDED
        if (r_noborder->current.enabled)
        {
            style = WS_VISIBLE | WS_POPUP;
        }
        else
        {
            style = WS_VISIBLE | WS_CAPTION | WS_SYSMENU;
        }
#else
        style = WS_VISIBLE | WS_CAPTION | WS_SYSMENU;
#endif
    }
    rc.left = 0;
    rc.right = wndParms->displayWidth;
    rc.top = 0;
    rc.bottom = wndParms->displayHeight;
    AdjustWindowRectEx(&rc, style, 0, exStyle);
    hinst = GetModuleHandleA(0);
    wndParms->hwnd = CreateWindowExA(
        exStyle,
        "CoD4",
        "Call of Duty 4",
        style,
        wndParms->x,
        wndParms->y,
        rc.right - rc.left,
        rc.bottom - rc.top,
        0,
        0,
        hinst,
        0);
    if (wndParms->hwnd)
    {
        Com_Printf(8, "Game window successfully created.\n");
        return 1;
    }
    else
    {
        Com_Printf(8, "Couldn't create a window.\n");
        return 0;
    }
}

void __cdecl Sys_HideSplashWindow()
{
    if (g_splashWnd)
        ShowWindow(g_splashWnd, 0);
}

void __cdecl Sys_DestroySplashWindow()
{
    if (g_splashWnd)
    {
        Sys_HideSplashWindow();
        DestroyWindow(g_splashWnd);
        g_splashWnd = 0;
    }
}

char __cdecl R_CreateGameWindow(GfxWindowParms *wndParms)
{
    if (!R_CreateWindow(wndParms))
        return 0;
    if (!R_InitHardware(wndParms))
        return 0;
    dx.targetWindowIndex = 0;
    ShowWindow(wndParms->hwnd, 5);
    Sys_HideSplashWindow();
    return 1;
}

void R_LoadGraphicsAssets()
{
    XZoneInfo zoneInfo[6]{ 0 }; // [esp+0h] [ebp-50h] BYREF
    unsigned int zoneCount; // [esp+4Ch] [ebp-4h]

    zoneInfo[0].name = gfxCfg.codeFastFileName;
    zoneInfo[0].allocFlags = 2;
    zoneInfo[0].freeFlags = 0;
    zoneCount = 1;

    if (gfxCfg.localizedCodeFastFileName)
    {
        zoneInfo[zoneCount].name = gfxCfg.localizedCodeFastFileName;
        zoneInfo[zoneCount].allocFlags = 0;
        zoneInfo[zoneCount].freeFlags = 0;
        zoneCount++;
    }
    if (gfxCfg.uiFastFileName)
    {
        zoneInfo[zoneCount].name = gfxCfg.uiFastFileName;
        zoneInfo[zoneCount].allocFlags = 8;
        zoneInfo[zoneCount].freeFlags = 0;
        zoneCount++;
    }

    zoneInfo[zoneCount].name = gfxCfg.commonFastFileName;
    zoneInfo[zoneCount].allocFlags = 4;
    zoneInfo[zoneCount].freeFlags = 0;
    zoneCount++;

    if (gfxCfg.localizedCommonFastFileName)
    {
        zoneInfo[zoneCount].name = gfxCfg.localizedCommonFastFileName;
        zoneInfo[zoneCount].allocFlags = 1;
        zoneInfo[zoneCount].freeFlags = 0;
        zoneCount++;
    }

    if (gfxCfg.modFastFileName)
    {
        zoneInfo[zoneCount].name = gfxCfg.modFastFileName;
        zoneInfo[zoneCount].allocFlags = 16;
        zoneInfo[zoneCount].freeFlags = 0;
        zoneCount++;
    }

    DB_LoadXAssets(zoneInfo, zoneCount, 0);
}

void __cdecl R_UpdateGpuSyncType()
{
    int integer; // [esp+0h] [ebp-4h]

    if (r_multiGpu->current.enabled)
        integer = 0;
    else
        integer = r_gpuSync->current.integer;
    dx.gpuSync = integer;
}

void __cdecl R_FinishAttachingToWindow(const GfxWindowParms *wndParms)
{
    if (dx.windowCount)
        MyAssertHandler(
            ".\\r_init.cpp",
            918,
            0,
            "%s\n\t(dx.windowCount) = %i",
            "(dx.windowCount >= 0 && dx.windowCount < ((123987 / ((((0) ? (123987) : (-123987)) * ((0) == 0 || (0) == 1))) == 1"
            "23987 / (123987)) ? 5 : 1))",
            dx.windowCount);
    iassert( dx.windows[dx.windowCount].swapChain );
    dx.windows[dx.windowCount].hwnd = wndParms->hwnd;
    dx.windows[dx.windowCount].width = wndParms->displayWidth;
    dx.windows[dx.windowCount++].height = wndParms->displayHeight;
}

char __cdecl R_InitHardware(const GfxWindowParms *wndParms)
{
    unsigned int workerIndex; // [esp+4h] [ebp-4h]

    if (!R_CreateDevice(wndParms))
        return 0;
    if (IsFastFileLoad())
        R_LoadGraphicsAssets();
    R_UpdateGpuSyncType();
    R_StoreWindowSettings(wndParms);
    RB_InitSceneViewport();
    KISAK_NULLSUB();
    if (!R_CreateForInitOrReset())
        return 0;
    R_Cinematic_Init();
    Com_Printf(8, "Setting initial state...\n");
    RB_SetInitialState();
    R_InitGamma();
    R_InitScene();
    R_InitSystems();
    KISAK_NULLSUB();
    R_FinishAttachingToWindow(wndParms);
    for (workerIndex = 0; workerIndex < 2; ++workerIndex)
    {
        iassert( r_smp_worker_thread[workerIndex] );
        Dvar_ClearModified((dvar_s*)r_smp_worker_thread[workerIndex]);
        if (r_smp_worker_thread[workerIndex]->current.enabled)
            Sys_ResumeThread((ThreadContext_t)(workerIndex + 2));
    }
    return 1;
}

void __cdecl R_StoreWindowSettings(const GfxWindowParms *wndParms)
{
    const char *v1; // eax
    float v2; // [esp+18h] [ebp-20h]
    int monitorHeight; // [esp+28h] [ebp-10h]
    int monitorWidth; // [esp+2Ch] [ebp-Ch]

    iassert( r_aspectRatio );
    vidConfig.sceneWidth = wndParms->sceneWidth;
    vidConfig.sceneHeight = wndParms->sceneHeight;
    vidConfig.displayWidth = wndParms->displayWidth;
    vidConfig.displayHeight = wndParms->displayHeight;
    vidConfig.displayFrequency = wndParms->hz;
    vidConfig.isFullscreen = wndParms->fullscreen;
    switch (r_aspectRatio->current.integer)
    {
    case 0:
        if (vidConfig.isFullscreen && dx.adapterNativeIsValid)
        {
            monitorWidth = dx.adapterNativeWidth;
            monitorHeight = dx.adapterNativeHeight;
        }
        else
        {
            monitorWidth = vidConfig.displayWidth;
            monitorHeight = vidConfig.displayHeight;
        }
        if ((int)((float)monitorHeight * 16.0f / (float)monitorWidth) == 10)
        {
            vidConfig.aspectRatioWindow = 1.6f;
        }
        else if ((int)((float)monitorHeight * 16.0f / (float)monitorWidth) >= 10)
        {
            vidConfig.aspectRatioWindow = 1.3333334f;
        }
        else
        {
            vidConfig.aspectRatioWindow = 1.7777778f;
        }
        break;
    case 1:
        vidConfig.aspectRatioWindow = 1.3333334f;
        break;
    case 2:
        vidConfig.aspectRatioWindow = 1.6f;
        break;
    case 3:
        vidConfig.aspectRatioWindow = 1.7777778f;
        break;
    default:
        if (!alwaysfails)
        {
            v1 = va("unhandled case, aspectRatio = %i\n", r_aspectRatio->current.integer);
            MyAssertHandler(".\\r_init.cpp", 470, 1, v1);
        }
        break;
    }
    iassert( com_wideScreen );
    Dvar_SetBool((dvar_s *)com_wideScreen, vidConfig.aspectRatioWindow != 1.333333373069763f);
    vidConfig.aspectRatioScenePixel = (float)vidConfig.sceneHeight
        * vidConfig.aspectRatioWindow
        / (float)vidConfig.sceneWidth;
    if (vidConfig.isFullscreen)
        vidConfig.aspectRatioDisplayPixel = (float)dx.adapterFullscreenHeight
        * vidConfig.aspectRatioWindow
        / (float)dx.adapterFullscreenWidth;
    else
        vidConfig.aspectRatioDisplayPixel = 1.0f;
}

void R_InitGamma()
{
    Dvar_SetModified((dvar_s*)r_gamma);
}

char __cdecl R_CreateForInitOrReset()
{
    const char *v0; // eax
    int hr; // [esp+8h] [ebp-8h]
    unsigned int fenceIter; // [esp+Ch] [ebp-4h]

    Com_Printf(8, "Initializing render targets...\n");
    R_InitRenderTargets();
    if (!g_allocateMinimalResources)
    {
        R_InitRenderBuffers();
        R_InitModelLightingImage();
        Com_Printf(8, "Initializing static model cache...\n");
        R_InitStaticModelCache();
    }
    Com_Printf(8, "Initializing dynamic buffers...\n");
    R_CreateDynamicBuffers();
    if (!g_allocateMinimalResources)
    {
        Com_Printf(8, "Initializing particle cloud buffer...\n");
        R_CreateParticleCloudBuffer();
    }
    Com_Printf(8, "Creating Direct3D queries...\n");
    dx.nextFence = 0;
    dx.flushGpuQueryIssued = 0;
    dx.flushGpuQueryCount = 0;

    hr = dx.device->CreateQuery(D3DQUERYTYPE_EVENT, &dx.flushGpuQuery);
    if (hr >= 0)
    {
        for (fenceIter = 0; fenceIter < 8; ++fenceIter)
        {
            hr = dx.device->CreateQuery(D3DQUERYTYPE_EVENT, &dx.fencePool[fenceIter]);

            if (hr < 0)
                goto LABEL_6;
        }
        if (!g_allocateMinimalResources)
        {
            RB_AllocSunSpriteQueries();
            gfxAssets.pixelCountQuery = RB_HW_AllocOcclusionQuery();
        }
        return 1;
    }
    else
    {
    LABEL_6:
        v0 = R_ErrorDescription(hr);
        Com_Printf(8, "Event query creation failed: %s (0x%08x)\n", v0, hr);
        return 0;
    }
}

IDirect3DQuery9 *__cdecl RB_HW_AllocOcclusionQuery()
{
    const char *v0; // eax
    int hr; // [esp+0h] [ebp-8h]
    IDirect3DQuery9 *query; // [esp+4h] [ebp-4h] BYREF

    hr = dx.device->CreateQuery(D3DQUERYTYPE_OCCLUSION, &query);
    if (hr >= 0)
        return query;
    v0 = R_ErrorDescription(hr);
    Com_Printf(8, "Occlusion query creation failed: %s (0x%08x)\n", v0, hr);
    return 0;
}

char __cdecl R_CreateDevice(const GfxWindowParms *wndParms)
{
    const char *v1; // eax
    _D3DPRESENT_PARAMETERS_ d3dpp; // [esp+0h] [ebp-44h] BYREF
    HWND__ *hwnd; // [esp+38h] [ebp-Ch]
    HRESULT hr; // [esp+3Ch] [ebp-8h]
    unsigned int behavior; // [esp+40h] [ebp-4h]

    iassert( wndParms );
    iassert( dx.windowCount == 0 );
    iassert( wndParms->hwnd );
    hwnd = wndParms->hwnd;
    iassert( dx.device == NULL );
    dx.depthStencilFormat = (D3DFORMAT)R_GetDepthStencilFormat(D3DFMT_A8R8G8B8);
    R_SetD3DPresentParameters(&d3dpp, wndParms);
    behavior = 70;
    hr = R_CreateDeviceInternal(hwnd, 0x46u, &d3dpp);
    r_glob.haveThreadOwnership = 1;
    if (hr >= 0)
    {
        iassert( dx.device );
        dx.deviceLost = 0;
        return 1;
    }
    else
    {
        v1 = R_ErrorDescription(hr);
        Com_Printf(8, "Couldn't create a Direct3D device: %s\n", v1);
        return 0;
    }
}

void __cdecl R_SetD3DPresentParameters(_D3DPRESENT_PARAMETERS_ *d3dpp, const GfxWindowParms *wndParms)
{
    iassert( d3dpp );
    iassert( wndParms );
    R_SetupAntiAliasing(wndParms);
    memset((unsigned __int8 *)d3dpp, 0, sizeof(_D3DPRESENT_PARAMETERS_));
    d3dpp->BackBufferWidth = wndParms->displayWidth;
    d3dpp->BackBufferHeight = wndParms->displayHeight;
    d3dpp->BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpp->BackBufferCount = 1;
    d3dpp->MultiSampleType = dx.multiSampleType;
    d3dpp->MultiSampleQuality = dx.multiSampleQuality;
    d3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp->EnableAutoDepthStencil = 0;
    d3dpp->AutoDepthStencilFormat = dx.depthStencilFormat;
    d3dpp->PresentationInterval = r_vsync->current.enabled ? 1 : 0x80000000;
    iassert( wndParms->hwnd );
    d3dpp->hDeviceWindow = wndParms->hwnd;
    d3dpp->Flags = 0;
    if (wndParms->fullscreen)
    {
        d3dpp->Windowed = 0;
        d3dpp->FullScreen_RefreshRateInHz = wndParms->hz;
    }
    else
    {
        d3dpp->Windowed = 1;
        d3dpp->FullScreen_RefreshRateInHz = 0;
    }
}

void __cdecl R_SetupAntiAliasing(const GfxWindowParms *wndParms)
{
    _D3DMULTISAMPLE_TYPE multiSampleCount; // [esp+0h] [ebp-Ch]
    DWORD qualityLevels; // [esp+8h] [ebp-4h] BYREF

    iassert( wndParms );
    if (wndParms->aaSamples < 1 || wndParms->aaSamples > 16)
        MyAssertHandler(
            ".\\r_init.cpp",
            248,
            0,
            "wndParms->aaSamples not in [1, 16]\n\t%i not in [%i, %i]",
            wndParms->aaSamples,
            1,
            16);
    if (r_reflectionProbeGenerate->current.enabled)
        multiSampleCount = D3DMULTISAMPLE_NONMASKABLE;
    else
        multiSampleCount = (_D3DMULTISAMPLE_TYPE)wndParms->aaSamples;
    while (multiSampleCount > D3DMULTISAMPLE_NONMASKABLE)
    {
        dx.multiSampleType = multiSampleCount;
        if (dx.d3d9->CheckDeviceMultiSampleType(
            0,
            D3DDEVTYPE_HAL,
            D3DFMT_A8R8G8B8,
            !wndParms->fullscreen,
            multiSampleCount,
            &qualityLevels) >= 0)
        {
            Com_Printf(8, "Using %ix anti-aliasing\n", multiSampleCount);
            dx.multiSampleQuality = qualityLevels - 1;
            return;
        }
        multiSampleCount = (_D3DMULTISAMPLE_TYPE)((int)multiSampleCount - 1);
    }
    dx.multiSampleType = D3DMULTISAMPLE_NONE;
    dx.multiSampleQuality = 0;
}

bool __cdecl R_GetMonitorDimensions(int *width, int *height)
{
    tagMONITORINFO mi; // [esp+0h] [ebp-2Ch] BYREF
    HMONITOR__ *adapterMonitor; // [esp+28h] [ebp-4h]

    adapterMonitor = dx.d3d9->GetAdapterMonitor(dx.adapterIndex);
    mi.cbSize = 40;
    if (GetMonitorInfoA(adapterMonitor, &mi))
    {
        *width = mi.rcMonitor.right - mi.rcMonitor.left;
        *height = mi.rcMonitor.bottom - mi.rcMonitor.top;
        return 1;
    }
    else
    {
        *width = GetSystemMetrics(0);
        *height = GetSystemMetrics(1);
        return *width > 0 && *height > 0;
    }
}

HRESULT __cdecl R_CreateDeviceInternal(HWND__ *hwnd, unsigned int behavior, _D3DPRESENT_PARAMETERS_ *d3dpp)
{
    _D3DDEVTYPE DeviceType; // eax
    _D3DDISPLAYMODE getModeResult; // [esp+4h] [ebp-18h] BYREF
    HRESULT hr; // [esp+14h] [ebp-8h]
    int attempt; // [esp+18h] [ebp-4h]

    Com_Printf(8, "Creating Direct3D device...\n");
    attempt = 0;
    while (1)
    {
        dx.adapterNativeIsValid = R_GetMonitorDimensions(&dx.adapterNativeWidth, &dx.adapterNativeHeight);
        DeviceType = (_D3DDEVTYPE)R_GetDeviceType();
        hr = dx.d3d9->CreateDevice(dx.adapterIndex, DeviceType, hwnd, behavior, d3dpp, &dx.device);
        if (hr >= 0)
            break;
        Sleep(100);
        if (++attempt == 20)
        {
            if (!dx.adapterIndex)
                return hr;
            dx.adapterIndex = 0;
            return R_CreateDeviceInternal(hwnd, behavior, d3dpp);
        }
    }
    if (dx.d3d9->GetAdapterDisplayMode(dx.adapterIndex, &getModeResult) < 0)
    {
        dx.adapterFullscreenWidth = d3dpp->BackBufferWidth;
        dx.adapterFullscreenHeight = d3dpp->BackBufferHeight;
    }
    else
    {
        dx.adapterFullscreenWidth = getModeResult.Width;
        dx.adapterFullscreenHeight = getModeResult.Height;
    }
    return hr;
}

int __cdecl R_GetDeviceType()
{
    _D3DADAPTER_IDENTIFIER9 id; // [esp+18h] [ebp-458h] BYREF

    if ((dx.d3d9->GetAdapterIdentifier)(
        dx.adapterIndex,
        0,
        &id) >= 0
        && !strcmp(id.Description, "NVIDIA NVPerfHUD"))
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

bool __cdecl R_SetCustomResolution(GfxWindowParms *wndParms)
{
    int monitorHeight; // [esp+0h] [ebp-8h] BYREF
    int monitorWidth; // [esp+4h] [ebp-4h] BYREF

    if (sscanf(r_customMode->current.string, "%ix%i", &wndParms->displayWidth, &wndParms->displayHeight) != 2)
        return 0;
    return !R_GetMonitorDimensions(&monitorWidth, &monitorHeight)
        || wndParms->displayWidth <= monitorWidth && wndParms->displayHeight <= monitorHeight;
}

const char *__cdecl R_ClosestRefreshRateForMode(unsigned int width, unsigned int height, int refreshRate)
{
    const char *v4; // eax
    int top; // [esp+0h] [ebp-10h]
    int bot; // [esp+4h] [ebp-Ch]
    const char *comparison; // [esp+8h] [ebp-8h]
    int mid; // [esp+Ch] [ebp-4h]

    bot = 0;
    top = dx.displayModeCount - 1;
    while (bot <= top)
    {
        mid = (bot + top) / 2;
        comparison = (const char *)(dx.displayModes[mid].Width - width);
        if (!comparison)
        {
            comparison = &dx.resolutionNameTable[4 * mid - 1023][-(int)height];
            if (!comparison)
            {
                comparison = &dx.resolutionNameTable[4 * mid - 1022][-refreshRate];
                if (!comparison)
                    return (const char *)refreshRate;
            }
        }
        if ((int)comparison >= 0)
            top = mid - 1;
        else
            bot = mid + 1;
    }
    iassert( (top >= 0) );
    iassert( top == bot - 1 );
    if (dx.displayModes[top].Width == width && dx.resolutionNameTable[4 * top - 1023] == (const char *)height)
        return dx.resolutionNameTable[4 * top - 1022];
    if (dx.displayModes[bot].Width != width || dx.resolutionNameTable[4 * bot - 1023] != (const char *)height)
    {
        v4 = va(
            "%i = (%i %i), %i = (%i %i), want (%i %i)",
            top,
            dx.displayModes[top].Width,
            dx.resolutionNameTable[4 * bot - 1023],
            bot,
            dx.displayModes[bot].Width,
            dx.resolutionNameTable[4 * bot - 1023],
            width,
            height);
        MyAssertHandler(
            ".\\r_init.cpp",
            1706,
            0,
            "%s\n\t%s",
            "dx.displayModes[bot].Width == width && dx.displayModes[bot].Height == height",
            v4);
    }
    return dx.resolutionNameTable[4 * bot - 1022];
}

void __cdecl R_SetWndParms(GfxWindowParms *wndParms)
{
    const char *resolutionString; // [esp+0h] [ebp-Ch]
    int refreshRate; // [esp+4h] [ebp-8h] BYREF
    const char *refreshRateString; // [esp+8h] [ebp-4h]

    wndParms->fullscreen = Dvar_GetBool("r_fullscreen");
    if (wndParms->fullscreen || !R_SetCustomResolution(wndParms))
    {
        resolutionString = Dvar_EnumToString(r_mode);
        sscanf(resolutionString, "%ix%i", &wndParms->displayWidth, &wndParms->displayHeight);
    }
    wndParms->sceneWidth = wndParms->displayWidth;
    wndParms->sceneHeight = wndParms->displayHeight;
    if (wndParms->fullscreen)
    {
        refreshRateString = Dvar_EnumToString(r_displayRefresh);
        sscanf(refreshRateString, "%i Hz", &refreshRate);
        wndParms->hz = (int)R_ClosestRefreshRateForMode(wndParms->displayWidth, wndParms->displayHeight, refreshRate);
    }
    else
    {
        wndParms->hz = 60;
    }
    wndParms->x = Dvar_GetInt("vid_xpos");
    wndParms->y = Dvar_GetInt("vid_ypos");
    wndParms->hwnd = 0;
    wndParms->aaSamples = r_aaSamples->current.integer;
}

void R_Register()
{
    R_RegisterDvars();
    R_RegisterCmds();
}

void R_InitGlobalStructs()
{
    memset((unsigned __int8 *)&rg, 0, sizeof(rg));
    memset((unsigned __int8 *)&rgp, 0, sizeof(rgp));
    RB_InitBackendGlobalStructs();
    rg.identityPlacement.base.quat[0] = 0.0;
    rg.identityPlacement.base.quat[1] = 0.0;
    rg.identityPlacement.base.quat[2] = 0.0;
    rg.identityPlacement.base.quat[3] = 1.0;
    rg.identityPlacement.base.origin[0] = 0.0;
    rg.identityPlacement.base.origin[1] = 0.0;
    rg.identityPlacement.base.origin[2] = 0.0;
    rg.identityPlacement.scale = 1.0;
    MatrixIdentity44(rg.identityViewParms.viewMatrix.m);
    MatrixIdentity44(rg.identityViewParms.projectionMatrix.m);
    MatrixIdentity44(rg.identityViewParms.viewProjectionMatrix.m);
    MatrixIdentity44(rg.identityViewParms.inverseViewProjectionMatrix.m);
}

void __cdecl R_EndRegistration()
{
    iassert( rg.registered );
    KISAK_NULLSUB();
    if (!IsFastFileLoad())
    {
        R_SyncRenderThread();
        RB_TouchAllImages();
    }
}

void __cdecl R_TrackStatistics(trStatistics_t *stats)
{
    rg.stats = stats;
}

void __cdecl R_UpdateTeamColors(int team, const float *color_allies, const float *color_axis)
{
    rg.team = team;
    Byte4PackRgba(color_allies, (unsigned __int8 *)&rg.color_allies);
    Byte4PackRgba(color_axis, (unsigned __int8 *)&rg.color_axis);
}

void __cdecl R_ConfigureRenderer(const GfxConfiguration *config)
{
    SetGfxConfig(config);
    R_InitRenderCommands();
}

void __cdecl R_ComErrorCleanup()
{
    iassert( Sys_IsMainThread() );
    R_AbortRenderCommands();
    R_SyncRenderThread();
    if (dx.inScene)
    {
        //((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *))dx.device->EndScene)(dx.device, dx.device);
        dx.device->EndScene();
        dx.inScene = 0;
    }
}

bool __cdecl R_CanRecoverLostDevice()
{
    HRESULT hr; // [esp+0h] [ebp-4h]

    iassert( dx.device );
    iassert( dx.deviceLost );
    hr = dx.device->TestCooperativeLevel();
    iassert( hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICENOTRESET );
    return hr != -2005530520;
}

void R_ReleaseForShutdownOrReset()
{
    IDirect3DSurface9 *pixelCountQuery; // [esp+0h] [ebp-18h]
    IDirect3DSurface9 *v1; // [esp+4h] [ebp-14h]
    IDirect3DSurface9 *var; // [esp+8h] [ebp-10h]
    IDirect3DSwapChain9 *varCopy; // [esp+Ch] [ebp-Ch]
    unsigned int fenceIter; // [esp+10h] [ebp-8h]
    int windowIndex; // [esp+14h] [ebp-4h]

    for (windowIndex = 0; windowIndex < dx.windowCount; ++windowIndex)
    {
        do
        {
            if (r_logFile)
            {
                if (r_logFile->current.integer)
                    RB_LogPrint("dx.windows[windowIndex].swapChain->Release()\n");
            }
            varCopy = dx.windows[windowIndex].swapChain;
            dx.windows[windowIndex].swapChain = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(
                (IDirect3DSurface9 *)varCopy,
                "dx.windows[windowIndex].swapChain",
                ".\\r_init.cpp",
                1009);
        } while (alwaysfails);
    }
    R_ShutdownRenderTargets();
    R_ShutdownModelLightingImage();
    R_ShutdownStaticModelCache();
    R_DestroyDynamicBuffers();
    R_DestroyParticleCloudBuffer();
    if (!g_allocateMinimalResources)
        R_ShutdownRenderBuffers();
    iassert( !gfxBuf.smodelCacheVb );
    if (dx.flushGpuQuery)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.flushGpuQuery->Release()\n");
            var = (IDirect3DSurface9 *)dx.flushGpuQuery;
            dx.flushGpuQuery = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(var, "dx.flushGpuQuery", ".\\r_init.cpp", 1026);
        } while (alwaysfails);
    }
    for (fenceIter = 0; fenceIter < 8; ++fenceIter)
    {
        if (dx.fencePool[fenceIter])
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("dx.fencePool[fenceIter]->Release()\n");
                v1 = (IDirect3DSurface9 *)dx.fencePool[fenceIter];
                dx.fencePool[fenceIter] = 0;
                R_ReleaseAndSetNULL<IDirect3DDevice9>(v1, "dx.fencePool[fenceIter]", ".\\r_init.cpp", 1031);
            } while (alwaysfails);
        }
    }
    RB_FreeSunSpriteQueries();
    if (gfxAssets.pixelCountQuery)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("gfxAssets.pixelCountQuery->Release()\n");
            pixelCountQuery = (IDirect3DSurface9 *)gfxAssets.pixelCountQuery;
            gfxAssets.pixelCountQuery = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(pixelCountQuery, "gfxAssets.pixelCountQuery", ".\\r_init.cpp", 1039);
        } while (alwaysfails);
    }
}

void R_ResetDevice()
{
    const char *v0; // eax
    const char *v1; // eax
    _D3DPRESENT_PARAMETERS_ d3dpp; // [esp+0h] [ebp-64h] BYREF
    HRESULT hr; // [esp+38h] [ebp-2Ch]
    GfxWindowParms wndParms; // [esp+3Ch] [ebp-28h] BYREF

    wndParms.hwnd = dx.windows[0].hwnd;
    wndParms.x = 0;
    wndParms.y = 0;
    wndParms.displayWidth = dx.windows[0].width;
    wndParms.displayHeight = dx.windows[0].height;
    wndParms.sceneWidth = dx.windows[0].width;
    wndParms.sceneHeight = dx.windows[0].height;
    wndParms.hz = vidConfig.displayFrequency;
    wndParms.fullscreen = vidConfig.isFullscreen != 0;
    wndParms.aaSamples = r_aaSamples->current.integer;
    R_SetD3DPresentParameters(&d3dpp, &wndParms);
    R_ReleaseForShutdownOrReset();
    //hr = dx.device->Reset(dx.device, &d3dpp);
    hr = dx.device->Reset(&d3dpp);
    if (hr < 0)
    {
        v0 = R_ErrorDescription(hr);
        v1 = va("Couldn't reset a lost Direct3D device - IDirect3DDevice9::Reset returned 0x%08x (%s)", hr, v0);
        R_FatalInitError(v1);
    }
    dx.deviceLost = 0;
    if (!R_CreateForInitOrReset())
        R_FatalInitError("Couldn't reinitialize after a lost Direct3D device");
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &gfxCmdBufInput, 0);
    R_InitCmdBufState(&gfxCmdBufState);
    RB_InitSceneViewport();
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
}

char __cdecl R_RecoverLostDevice()
{
    int remoteScreenUpdateNesting; // [esp+0h] [ebp-4h]

    iassert( dx.device );
    iassert( dx.deviceLost );
    iassert( gfxBuf.dynamicVertexBuffer->buffer );
    iassert( gfxBuf.dynamicIndexBuffer->buffer );
    if (!R_CanRecoverLostDevice())
        return 0;
    Com_Printf(8, "Recovering lost device...\n");
    remoteScreenUpdateNesting = R_PopRemoteScreenUpdate();
    R_SyncRenderThread();
    R_Cinematic_BeginLostDevice();
    DB_BeginRecoverLostDevice();
    R_ResetModelLighting();
    R_ReleaseLostImages();
    Material_ReleaseAll();
    R_ReleaseWorld();
    R_ResetDevice();
    R_ReloadWorld();
    Material_ReloadAll();
    R_ReloadLostImages();
    dx.sunSpriteSamples = RB_CalcSunSpriteSamples();
    DB_EndRecoverLostDevice();
    R_Cinematic_EndLostDevice();
    R_PushRemoteScreenUpdate(remoteScreenUpdateNesting);
    Com_Printf(8, "Finished recovering lost device.\n");
    return 1;
}

bool R_CheckLostDevice()
{
    if (!dx.device)
        return false;
    
    if (!dx.deviceLost)
    {
        HRESULT hr = dx.device->TestCooperativeLevel();
        if (hr != D3DERR_DEVICELOST && hr != D3DERR_DEVICENOTRESET)
            return true;

        R_SyncRenderThread();
        dx.deviceLost = 1;
    }

    if (Sys_IsMainThread())
        R_RecoverLostDevice();
    
    return false;
}

void __cdecl R_MakeDedicated(const GfxConfiguration *config)
{
    SetGfxConfig(config);
    if (!r_loadForRenderer)
        R_RegisterDvars();
    Dvar_SetBool(r_loadForRenderer, 0);
    Dvar_MakeLatchedValueCurrent((dvar_s*)r_loadForRenderer);
    R_LoadGraphicsAssets();
}

int R_IsHiDef()
{
#ifdef KISAK_SP
    return vidConfig.isHiDef;
#elif KISAK_MP
    return 1;
#endif
}