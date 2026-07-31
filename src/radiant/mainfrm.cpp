#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// CMainFrame - the editor shell: the five render windows + console strip in a QE4 layout,
// the IDR_MENU_QUAKE3 menu/accelerators, the status bar, and the menu command wrappers.
// Renderer bring-up is R_BeginRegistrationInternal (0x416510).

#include "stdafx.h"
#include <ctime>                       // clock() — RoutineProcessing camera-fly dtime (0x421a90)
#include "mainfrm.h"
#include "qe3.h"                       // g_qeglobals, qeglobals_t, grid_sizes
#include "prefs.h"                     // g_PrefsDlg, Prefs_ShowDialog/SavePrefs
#include "xywnd.h"                     // ED_VIEW_*
#include <qcommon/qcommon.h>           // Dvar_Init
#include <qcommon/cmd.h>               // Cbuf_Init, Cmd_Init
#include <qcommon/threads.h>           // THREAD_CONTEXT_COUNT
#include <win32/win_local.h>           // CRITSECT_COUNT (radiant-safe; cmd.cpp includes it too)
#include <universal/com_memory.h>      // Com_InitHunkMemory
#include <universal/com_files.h>       // FS_InitFilesystem
#include <gfx_d3d/r_init.h>            // R_InitEditor, R_InitRendererForWindow
#include <gfx_d3d/r_rendercmds.h>      // R_InitRenderCommands
#include <gfx_d3d/r_material.h>        // Material (R_BeginRegistrationInternal return)

extern CMainFrame *g_pParentWnd;             // engine_stubs.cpp
extern void Radiant_RegisterGroupCDvars();   // engine_stubs.cpp
extern void Sys_InitializeCriticalSections();// universal/win_common.cpp (decl lives in win32/win_local.h, not radiant-safe)
extern void track_init();                    // qcommon/mem_track.h
extern void SL_Init();                        // script/scr_stringlist.cpp (also inits the script memory tree)

// The real renderer bootstrap (gfxwrapper.cpp, IDB 0x416510). Asserts d_hwndCamera/XY/Z/
// Texture + lyrMtlWndGlob.layerList, attaches the device to each, registers fonts/qerfont
// (g_qeglobals.d_font_list) + white_tools/$opaque/$additive.
extern Material *R_BeginRegistrationInternal();

// Map / status pipeline.
extern void       Load_Materials();                              // texwnd.cpp (0x45ae40) bulk material load
extern void       Get_MaterialNames();                            // qe3.cpp (0x45aaa0), Load_Textures head
extern unsigned long FillTextureMenu();                          // texwnd.cpp (0x45b260) Usage/Locale/Surface filter submenus
extern LRESULT    Texture_ShowAll();                             // texwnd.cpp (0x45b730) un-hide all materials
extern void       Texture_ResetPosition();                       // texwnd.cpp (0x45b650) reset texture-browser scroll
extern void       Map_LoadFromFile( const char *path );          // map.cpp (0x486680)
extern void       Map_NewMap();                                  // map.cpp (0x486110)
extern void       Map_New();                                     // entity.cpp (0x4870C0) File→New
extern entity_s  *world_entity;                                  // map.cpp (0x25D5B30)
extern void       Prefab_LevelBack();                            // map.cpp (0x489D50)
extern void       Map_SaveFile( const char *path, char a1, char a2 ); // map.cpp
extern eclass_t  *Eclass_ForName( int has_brushes, const char *name ); // eclass.cpp
extern selbrush_t active_brushes;                                // map.cpp (0x23F189C)
extern void       QE_CountBrushesAndUpdateStatusBar();           // qe3.cpp
extern void       Entity_UpdateSelection();                      // win_ent.cpp (UpdateSelection(-1,NULL))
extern void       Z_CenterOnMap();                               // z.cpp
extern void       Cam_CenterOnMap( CCamWnd *cam );               // camwnd.cpp
extern void       Undo_Undo();                                   // undo.cpp
extern void       Undo_Redo();                                   // undo.cpp
extern int        g_nUpdateBits;                                 // engine_stubs.cpp (0x25D5A74)

// Console sink — the binary's CMainFrame::OnCreate installs console_print as the
// global console_stuff callback (console_stuff = &console_print, 0x420A54), so every
// editor-log Com_PrintMessage / Com_PrintError / R_Warn line lands in the console
// pane (d_hwndEdit).  cmdlib.cpp owns console_stuff + SetConsoleHandler (0x40A9E0);
// console_print lives in win_qe3.cpp.
extern void       console_print( const char *fmt, va_list args );          // win_qe3.cpp (0x499D80)
extern void       SetConsoleHandler( void ( *fn )( const char *, va_list ) );// cmdlib.cpp (0x40A9E0)
extern BOOL       LoadRegistryInfo( const char *pszName, void *pvBuf, long *plSize ); // win_qe3.cpp 0x4999C0

// Editor trace log (%TEMP%\radiant_firstlight.log) - append+flush+close per call so the
// last line survives a crash inside a window callback.  Startup/error lines only.
void Radiant_FL_Log( const char *fmt, ... )
{
    char buf[1024];
    va_list ap; va_start( ap, fmt );
    _vsnprintf( buf, sizeof( buf ), fmt, ap );
    va_end( ap );
    char path[MAX_PATH], tmp[MAX_PATH];
    GetTempPathA( sizeof( tmp ), tmp );
    _snprintf( path, sizeof( path ), "%sradiant_firstlight.log", tmp );
    FILE *f = fopen( path, "a" );
    if ( f ) { fputs( buf, f ); fputc( '\n', f ); fclose( f ); }
}

static void Radiant_FL_LogReset()
{
    char path[MAX_PATH], tmp[MAX_PATH];
    GetTempPathA( sizeof( tmp ), tmp );
    _snprintf( path, sizeof( path ), "%sradiant_firstlight.log", tmp );
    FILE *f = fopen( path, "w" );
    if ( f ) { fputs( "=== radiant trace ===\n", f ); fclose( f ); }
}

// Resolve a code address to "module!0xoffset" - the module it belongs to (NOT necessarily
// the .exe) + its RVA within it.  scripts\radiant\symbolicate.ps1 resolves it via dbghelp.
static void Radiant_FL_ResolveAddr( void *addr, char *buf, size_t bufsz )
{
    HMODULE mod = NULL;
    if ( GetModuleHandleExA(
             GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
             (LPCSTR)addr, &mod ) && mod )
    {
        char modpath[MAX_PATH] = { 0 };
        const char *modname = "?";
        if ( GetModuleFileNameA( mod, modpath, sizeof( modpath ) ) )
        {
            const char *slash = strrchr( modpath, '\\' );
            modname = slash ? slash + 1 : modpath;
        }
        _snprintf( buf, bufsz, "%s!0x%X", modname,
            (unsigned)( (uintptr_t)addr - (uintptr_t)mod ) );
    }
    else
    {
        _snprintf( buf, bufsz, "%p!?? (no module)", addr );
    }
    buf[bufsz - 1] = 0;
}

// VEH: log the faulting address of hard exceptions (AV / illegal-instr / int3 / fastfail) as
// module!offset, the read/write target for AVs, and a CaptureStackBackTrace caller chain.
static LONG WINAPI Radiant_FL_Veh( EXCEPTION_POINTERS *ep )
{
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    if ( code == 0xC0000005 /*AV*/ || code == 0xC000001D /*illegal instr*/ ||
         code == 0x80000003 /*int3/breakpoint*/ || code == 0xC0000409 /*fastfail/stack overrun*/ )
    {
        void *addr = ep->ExceptionRecord->ExceptionAddress;
        char faultloc[320];
        Radiant_FL_ResolveAddr( addr, faultloc, sizeof( faultloc ) );

        if ( code == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2 )
        {
            // ExceptionInformation[0]: 0=read, 1=write, 8=DEP/execute; [1]=target VA.
            ULONG_PTR op = ep->ExceptionRecord->ExceptionInformation[0];
            ULONG_PTR va = ep->ExceptionRecord->ExceptionInformation[1];
            Radiant_FL_Log( "*** EXCEPTION 0x%08X at %s  (%s 0x%p) ***",
                code, faultloc, op == 1 ? "write" : op == 8 ? "exec" : "read", (void *)va );
        }
        else
        {
            Radiant_FL_Log( "*** EXCEPTION 0x%08X at %s ***", code, faultloc );
        }

        // Short backtrace; the first 2-3 frames are this VEH + KiUserExceptionDispatcher.
        void *frames[16] = { 0 };
        USHORT n = CaptureStackBackTrace( 0, 16, frames, NULL );
        Radiant_FL_Log( "    --- backtrace (%u frames) ---", (unsigned)n );
        for ( USHORT i = 0; i < n; ++i )
        {
            char fl[320];
            Radiant_FL_ResolveAddr( frames[i], fl, sizeof( fl ) );
            Radiant_FL_Log( "    [%2u] %p  %s", (unsigned)i, frames[i], fl );
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_WM_CLOSE()                       // ConfirmModified unsaved-changes guard on exit (0x422220)
    ON_WM_SETCURSOR()                   // splitter-bar resize cursor over the pane gutters
    ON_WM_LBUTTONDOWN()                 // splitter-bar drag start
    ON_WM_LBUTTONUP()                   // splitter-bar drag end
    ON_WM_MOUSEMOVE()                   // splitter-bar drag
    ON_WM_KEYDOWN()                     // dispatch editor hotkeys when the FRAME itself has focus
                                        // (e.g. right after a toggled inspector popup hides and
                                        //  returns focus to its owner frame) — else F/N/O/T would
                                        //  be dead until a view is re-focused by clicking.
    // File / Edit command wrappers.
    ON_COMMAND(ID_FILE_NEW,         OnFileNew)
    ON_COMMAND(ID_FILE_OPEN,        OnFileOpen)
    ON_COMMAND(ID_FILE_SAVE,        OnFileSave)
    ON_COMMAND(ID_FILE_SAVEAS_RAD,  OnFileSaveAs)
    ON_COMMAND(ID_FILE_EXIT_RAD,    OnFileExit)
    // Project (.prj) menu items (IDB CMainFrame AFX_MSGMAP).
    ON_COMMAND(32791, OnFileNewproject)      // File→New Project        (IDB 0x426E80)
    ON_COMMAND(32818, OnFileProjectsettings) // File→Project Settings   (IDB 0x428DE0)
    ON_COMMAND(35018, OnSetStartupProject)   // File→Set Startup Project(IDB 0x427010)
    ON_COMMAND_RANGE(8000, 8009, OnMru)      // File→Recent Files       (IDB OnMru 0x423FE0)
    ON_WM_DESTROY()                          // SaveMruInReg on shutdown (IDB OnDestroy 0x421C60)
    ON_COMMAND(ID_EDIT_UNDO,        OnEditUndo)
    ON_COMMAND(ID_EDIT_REDO,        OnEditRedo)
    ON_COMMAND(33039, OnEditCopybrush)      // Edit→Copy   (msgmap nID 0x810F, IDB 0x4286B0)
    ON_COMMAND(33040, OnEditPastebrush)     // Edit→Paste  (msgmap nID 0x8110, IDB 0x4286D0)
    // Grid menu (11 grid-size items, OnGrid1 ranged handler).
    ON_COMMAND_EX(35021, OnGridSize) ON_COMMAND_EX(35022, OnGridSize)
    ON_COMMAND_EX(35023, OnGridSize) ON_COMMAND_EX(35024, OnGridSize)
    ON_COMMAND_EX(35025, OnGridSize) ON_COMMAND_EX(35026, OnGridSize)
    ON_COMMAND_EX(35027, OnGridSize) ON_COMMAND_EX(35029, OnGridSize)
    ON_COMMAND_EX(35031, OnGridSize) ON_COMMAND_EX(35032, OnGridSize)
    ON_COMMAND_EX(35033, OnGridSize)

    // View->Zoom + Selection menu (raw menu IDs, matching radiant.rc).
    ON_COMMAND(32995, OnViewZoomin)        // View→Zoom→XY Zoom In
    ON_COMMAND(32996, OnViewZoomout)       // View→Zoom→XY Zoom Out
    ON_COMMAND(32968, OnView100)           // View→Zoom→XY 100%
    ON_COMMAND(33002, OnSelectionDeselect) // Selection→Deselect
    ON_COMMAND(33005, OnSelectionDragVertices) // Selection→Drag Vertices (vertex-edit mode)
    ON_COMMAND(33006, OnSelectionDragEdges) // Selection→Drag Edges (edge-edit mode)
    ON_COMMAND(32982, OnSelectionMakehollow) // Selection→CSG→Hollow
    ON_COMMAND(32927, OnSelectionCsgmerge) // Selection→CSG→Merge
    ON_COMMAND(33183, OnDropSelected)      // Selection→Drop to Floor (IDB OnDropSelected 0x425be0, nID 0x819F)

    // CLIPPER — View→Toggle Clipper / Clip selected / Flip Clip orientation.
    ON_COMMAND(32783, OnViewClipper)        // toggle clip mode
    ON_COMMAND(32795, OnClipSelected)       // commit the clip (Enter / Shift+Enter)
    ON_COMMAND(32796, OnFlipClip)           // flip kept side
    ON_COMMAND(32794, OnSplitSelected)      // Selection→Clipper→Split selected (Shift+Enter, 0x4271D0)
    ON_COMMAND(32809, OnSelectScale)        // Selection→Scale...                          (0x4283D0)
    // PHYSICS→Primitives (Cylinder / Box).
    ON_COMMAND(36113, OnMakePhysCylinder)   // Physics→Cylinder                            (0x4291D0)
    ON_COMMAND(36120, OnMakePhysBox)        // Physics→Box                                 (0x429200)

    // Region menu (whole category) + Edit->Leave Prefab.
    ON_COMMAND(32979, OnRegionOff)          // Region→Off
    ON_COMMAND(32980, OnRegionSetxy)        // Region→Set XY
    ON_COMMAND(33007, OnRegionSettallbrush) // Region→Set Tall Brush
    ON_COMMAND(32981, OnRegionSetbrush)     // Region→Set Brush
    ON_COMMAND(33044, OnRegionSetselection) // Region→Set Selected Brushes
    ON_COMMAND(33173, OnPrefabEnter)        // Edit→Enter Prefab (body in map.cpp)
    ON_COMMAND(33174, OnPrefabLeave)        // Edit→Leave Prefab (body in map.cpp)

    // Edit->Preferences + the Snap-to-grid / Texture-Lock toggles (persist via SavePrefs).
    ON_COMMAND(32784, OnPrefs)              // Edit→Preferences... (IDB OnPrefs 0x426950)
    ON_COMMAND(32793, OnSnaptogrid)         // Grid→Snap to grid  (IDB OnSnaptogrid 0x428380)
    ON_COMMAND(32785, OnToggleLockMoves)    // Textures→Texture Lock→Moves     (0x426b80)
    ON_COMMAND(32835, OnToggleLockRotations)// Textures→Texture Lock→Rotations (0x429230)
    ON_COMMAND(33237, OnToggleLockLightmap) // Textures→Texture Lock→Lightmaps (0x426bf0)
    ON_COMMAND(33041, OnTexturesInspector)  // Textures→Surface Inspector      (0x424b60)
    ON_COMMAND(33092, OnPatchInspector)     // Patch→Inspector (Shift+S)       (0x42b460)
    ON_COMMAND(32812, OnTextureReplaceall)  // Textures→Replace All (Find/Replace) (0x428b40)
    ON_COMMAND(33954, OnLayersDlg)          // Layers→Layers... toggle the Layers dialog (0x42bd10)
    ON_COMMAND(35001, OnSelectionAddToActiveLayer) // ctx-menu "Add selection to active layer" (CXYWnd 0x466930, nID 0x88B9)
    ON_COMMAND(36106, OnMiscDynEntities)    // Misc→Dyn Entities toggle the Dyn-Entity dialog (0x42bd90 / 0x8D0A)
    ON_COMMAND(33221, OnMiscVehicleGroup)   // Misc→Vehicle Group toggle the Vehicle-Group dialog (0x42bd50 / 0x81C5)
    ON_COMMAND(33240, OnReplaceModels)      // Replace Models toggle the Replace-Models tool dialog (0x42bf00 / 0x81D8)
    ON_COMMAND(33199, OnVertexEditDlg)      // Vertex Edit (accel 'G') toggle the Vertex-Edit dialog (0x42bcd0 / 0x81AF)
    ON_COMMAND(32786, OnEditMapinfo)        // Edit→Map Info... open the read-only Map Info dialog (0x426c60 / 0x8012)
    ON_COMMAND(32787, OnEditEntityinfo)     // Edit→Entity Info... open the entity-browser tree+K/V dialog (0x426d6f / 0x8013)

    // BRUSH → PRIMITIVES — reshape the selected brush into a cylinder / cone / sphere.
    ON_COMMAND(33034, OnBrushArbitrarysided)   // Brush→Arbitrary sided cylinder...
    ON_COMMAND(32833, OnBrushMakecone)         // Brush→Primitives→Cone...
    ON_COMMAND(32892, OnBrushPrimitivesSphere) // Brush→Primitives→Sphere...

    // SELECTION menu batch — forwards to already-ported select.cpp/entity.cpp cores.
    ON_COMMAND(33134, OnSelectConneted)        // Selection→Select Connected
    ON_COMMAND(33132, OnSelectionTargetname)   // Selection→Select Targetname
    ON_COMMAND(202,   OnSelectionClassname)    // Selection→Select Classname
    ON_COMMAND(33035, OnSelectionUngroupentity)// Selection→Ungroup entity
    ON_COMMAND(33042, OnSelectionMakeDetail)   // Selection→Make Detail
    ON_COMMAND(33043, OnSelectionMakeStructural)// Selection→Make Structural
    ON_COMMAND(33133, OnSelectionKeyValue)     // Selection→Select by Key/Value

    // REGION SELECTION — take the single selected brush as a box, reselect matching
    // brushes (cores ported this unit in select.cpp). Also the marquee drag's menu twins.
    ON_COMMAND(32984, OnSelectionCompleteTall) // Selection→Select Complete Tall  (Select_CompleteTall)
    ON_COMMAND(32983, OnSelectionPartialTall)  // Selection→Select Partial Tall   (Select_PartialTall)
    ON_COMMAND(32986, OnSelectionTouching)     // Selection→Select Touching       (Select_Touching_R)
    ON_COMMAND(33008, OnSelectionInside)       // Selection→Select Inside         (Select_Inside_R)

    // MENU-WRAPPER BATCH (wiring-only).
    ON_COMMAND(33003, OnSelectionDelete)       // Edit→Delete (Select_Delete + undo walk)
    ON_COMMAND(33220, OnSelectionAutoCaulk)    // Selection→CSG→Auto Caulk (Brush_AutoCaulk)
    ON_COMMAND(196,   OnSelectionMakeWeaponclip)     // Selection→Make Weapon Clip
    ON_COMMAND(197,   OnSelectionMakeNonColliding)   // Selection→Make Non-Colliding
    ON_COMMAND(33223, OnSelectionMakeSplitCoplanar)  // Selection→Make Split Coplanar Geo
    ON_COMMAND(33224, OnSelectionMakeDontSplitCoplanar)// Selection→Make Don't Split Coplanar Geo
    ON_COMMAND(32953, OnViewCenter)            // View→Center (camera angle snap)
    ON_COMMAND(32954, OnViewUpfloor)           // View→Up Floor   (Cam_ChangeFloor 1)
    ON_COMMAND(32955, OnViewDownfloor)         // View→Down Floor (Cam_ChangeFloor 0)
    ON_COMMAND(32974, OnTexturesShowinuse)     // Textures→Show In Use (Texture_ShowInuse)
    ON_COMMAND(32973, OnTexturesShowall)       // Textures→Show All / Ctrl-A (Texture_ShowAll)  (0x42b440)

    // INSPECTOR TAB SWITCHING (SetInspectorMode) — toggle the right-column inspector mode.
    ON_COMMAND(33017, OnViewEntity)            // View→Toggle→Entity View  (SetInspectorMode ENTITY)
    ON_COMMAND(33018, OnViewTextureMode)       // View→Toggle→Texture View (SetInspectorMode TEXTURE)
    ON_COMMAND(33016, OnViewConsole)           // View→Toggle→Console View (SetInspectorMode CONSOLE)
    ON_COMMAND(33104, OnFilterDlg)             // View→Filter Settings     (SetInspectorMode FILTER)

    // TEXTURES→Usage / Locale / Surface type filter submenus (built at startup by
    // FillTextureMenu).  IDB CMainFrame AFX_MSGMAP: WM_COMMAND ranges (nSig AfxSig_vw):
    //   60000..60255 → OnFilterUsage       (0x4243e0)
    //   60256..60511 → OnFilterLocale      (0x424400)
    //   60512..60767 → OnFilterSurfaceType (0x424420)
    ON_COMMAND_RANGE(60000, 60255, OnFilterUsage)
    ON_COMMAND_RANGE(60256, 60511, OnFilterLocale)
    ON_COMMAND_RANGE(60512, 60767, OnFilterSurfaceType)

    // SELECTION TRANSFORMS - Clone + Flip/Rotate X/Y/Z.
    ON_COMMAND(33001, OnSelectionClone)        // Selection→Clone (Clone_Selection)
    ON_COMMAND(32956, OnBrushFlipx)            // Brush→Flip→X  (DoFlip 0/"flip X")
    ON_COMMAND(32957, OnBrushFlipy)            // Brush→Flip→Y  (DoFlip 1/"flip Y")
    ON_COMMAND(32958, OnBrushFlipz)            // Brush→Flip→Z  (DoFlip 2/"flip Z")
    ON_COMMAND(32959, OnBrushRotatex)          // Brush→Rotate→X (Select_RotateAxis 0, 90)
    ON_COMMAND(32960, OnBrushRotatey)          // Brush→Rotate→Y (Select_RotateAxis 1, 90)
    ON_COMMAND(32961, OnBrushRotatez)          // Brush→Rotate→Z (Select_RotateAxis 2, 90)

    // TOOLBAR command handlers (IDR_TOOLBAR152); bodies below (search "TOOLBAR COMMAND HANDLERS").
    ON_COMMAND(33135, OnTextureFlipX)          // Brush_FlipTexture(0)
    ON_COMMAND(33136, OnTextureFlipY)          // Brush_FlipTexture(1)
    ON_COMMAND(33182, OnTextureRotate90)       // Brush_RotateTexture(90)
    ON_COMMAND(33238, OnEditLayerCycle)        // cycle current material layer 0/1/2
    ON_COMMAND(32936, OnToggleCameraMovementMode) // camera_mode 0/1/2 cycle
    ON_COMMAND(32817, OnViewCubicclipping)     // m_bCubicClipping toggle
    ON_COMMAND(32781, OnViewChange)            // XY view-type cycle
    ON_COMMAND(32810, OnSelectMouserotate)     // free mouse-rotation toggle
    ON_COMMAND(32813, OnSelectMousescale)      // mouse-scale mode toggle
    ON_COMMAND(32814, OnScalelockX)            // scale-lock X
    ON_COMMAND(32815, OnScalelockY)            // scale-lock Y
    ON_COMMAND(32816, OnScalelockZ)            // scale-lock Z
    ON_COMMAND(32852, OnDontselectcurve)       // m_bSelectCurves toggle
    ON_COMMAND(32857, OnPatchWireframe)        // patch_wireframe 0/1/2 cycle
    ON_COMMAND(32858, OnPatchWeld)             // g_bPatchWeld toggle
    ON_COMMAND(32865, OnPatchDrilldown)        // patch_drill_down toggle
    ON_COMMAND(33140, ToggleLockPatchVertMode) // bLockPatchVerts toggle
    ON_COMMAND(33139, ToggleUnlockPatchVertMode)// bUnlockPatchVerts toggle
    ON_COMMAND(33141, OnCycleTerrainEdge)      // sel_cycle_edge_direction_quad toggle
    ON_COMMAND(33138, OnToggleTextureAlphaRendering) // camera_masked toggle
    ON_COMMAND(33142, OnDisableSelectionOfEntities)  // entities_off toggle
    ON_COMMAND(33169, OnDisableSelectionOfSky) // sky_brush_off toggle
    ON_COMMAND(33144, OnToggleDrawSurfs)       // draw_toggle toggle
    ON_COMMAND(33156, OnSelectableModels)      // m_bSelectableModels toggle
    ON_COMMAND(33159, OnPlantModel)            // m_bDropModel toggle
    ON_COMMAND(33206, OnForceZeroDropHeight)   // m_bForceZeroDropHeight toggle
    ON_COMMAND(33195, OnOrientToFloor)         // m_bOrientModel toggle
    ON_COMMAND(33155, OnTolerantWeld)          // m_bTolerantWeld toggle
    ON_COMMAND(33207, OnVertSnapModel)         // m_bVertSnapModel toggle
    ON_COMMAND(33208, OnVertSnapBrush)         // m_bVertSnapBrush toggle
    ON_COMMAND(33209, OnVertSnapPrefab)        // m_bVertSnapPrefab toggle
    ON_COMMAND(32872, OnPatchRedisperse)       // Patch_InsDelToggle (redisperse mode)
    ON_COMMAND(33130, OnAdvancedEditDlg)       // Patch→Advanced Edit Dialog (terrain-paint settings)
    ON_COMMAND(36125, OnShowRegionsForSelected)// "show regions for selected" (Shift+F8, cmd 0x8D1D, IDB 0x4241E0)
    ON_COMMAND(33955, OnSetAsActiveLayer)       // Layers "Set as active layer" (cmd 0x84A3, IDB 0x42BFD0 — empty stub in binary)
    ON_COMMAND(35005, OnMiscCyclePreviewModels)// w_cyclePreviewMode entity model cycle
    ON_COMMAND(35042, OnDropSelectedRelativeZ) // relative-Z drop of selected curve points
    ON_COMMAND(32915, OnShowEntities)          // entity-display-mode popup (HandlePopup 160)
    ON_COMMAND(32909, OnViewEntitiesasBoundingbox)
    ON_COMMAND(32916, OnViewEntitiesasWireframe)
    ON_COMMAND(32911, OnViewEntitiesasSelectedwireframe)
    ON_COMMAND(32912, OnViewEntitiesasSelectedskinned)
    ON_COMMAND(32913, OnViewEntitiesasSkinned)
    ON_COMMAND(32914, OnViewEntitiesasSkinnedandboxed)

    // SMALL MODAL UTILITY DIALOGS (win_dlg.cpp — hand-built popups over ported cores).
    ON_COMMAND(33023, OnMiscFindbrush)         // Misc→Find brush (Select_ByEntityNumber)
    ON_COMMAND(33107, OnMiscGoToPosition)      // Misc→Go to position (camera/XY origin)
    ON_COMMAND(33186, OnMiscMayaExport)        // Misc→Maya Export (ExportToMaya — .mel generator)
    ON_COMMAND(33033, OnSelectionArbitraryrotation)// Selection→Rotate→Arbitrary (Select_RotateAxis)
    ON_COMMAND(33038, OnHelpAbout)             // Help→About (info popup)
    ON_COMMAND(36109, OnErrorFile)             // File→Error file (Pointfile_Errorfile toggle)
    ON_COMMAND(32967, OnPointfileOpen)         // File→Pointfile (IDB 0x423b20, 0x80C7)
    ON_COMMAND(33024, OnMiscNextleakspot)      // Misc→Next leak spot (IDB 0x424bc0, accel 'K')
    ON_COMMAND(33025, OnMiscPreviousleakspot)  // Misc→Previous leak spot (IDB 0x424be0, accel 'L')
    ON_COMMAND(33037, OnMiscPrintxy)           // Misc→Print XY View (IDB 0x424c00, WXY_Print)
    ON_COMMAND(33087, OnSelectionPrint)        // Selection→Print (IDB 0x429110, Brush_Print)

    // TEXTURES→Render Method — Material_SetMode(0/1/2) (now real; texwnd.cpp).
    // (These are the LAYER radio — a different feature from the 32990..32994 range below,
    //  which is the binary's genuine Render-Method radio.  Do not conflate them.)
    ON_COMMAND(33232, OnRenderMethodMaterial)  // Material
    ON_COMMAND(33233, OnRenderMethodLightmap)  // Lightmap
    ON_COMMAND(36100, OnRenderMethodSmoothing) // Smoothing
    // TEXTURES→Render Method radio (Wireframe/Fullbright/Normal-fake/View-fake/Case textures).
    // IDB CMainFrame AFX_MSGMAP ON_COMMAND_RANGE 32990..32994 → OnRendermethodCaseTextures
    // (0x4243D0) → Texture_SetMode(nID) (0x45A520, texwnd.cpp).
    ON_COMMAND_RANGE(32990, 32994, OnRendermethodCaseTextures)

    // TEXTURES→Texture Filter submenu (33226..33230) — each sets the "r_textureMode" dvar.
    ON_COMMAND(33226, OnTextureFilterNearest)     // Nearest      (0x424200)
    ON_COMMAND(33227, OnTextureFilterLinear)      // Linear       (0x424250)
    ON_COMMAND(33228, OnTextureFilterBilinear)    // Bilinear     (0x4242A0)
    ON_COMMAND(33229, OnTextureFilterTrilinear)   // Trilinear    (0x4242F0)
    ON_COMMAND(33230, OnTextureFilterAnisotropic) // Anisotropic  (0x424380)

    // TEXTURE REFRESH / RESOLUTION / WINDOW-SCALE - reload backend in gfx_d3d/r_image.cpp
    // (KISAK_RADIANT-gated).
    ON_COMMAND(33204, OnTextureRefresh)              // Textures→Refresh Textures (F5)  (0x428B50)
    ON_COMMAND_RANGE(36115, 36118, OnTextureResolution) // Textures→Texture Resolution Max/High/Normal/Low (0x424340)
    ON_COMMAND(32894, OnTexturesTexturewindowscale200)  // Texture Window Scale 200%    (0x42B020)
    ON_COMMAND(32895, OnTexturesTexturewindowscale100)  // 100%                          (0x42B000)
    ON_COMMAND(32896, OnTexturesTexturewindowscale50)   // 50%                           (0x42B060)
    ON_COMMAND(32897, OnTexturesTexturewindowscale25)   // 25%                           (0x42B040)
    ON_COMMAND(32898, OnTexturesTexturewindowscale10)   // 10%                           (0x42AFE0)

    // LAYERED MATERIALS — the authoring tool palette (layeredmaterialwnd.cpp).
    // IDB AFX_MSGMAP: cmd 35008 (F4 accel) → OnToggleLayeredMaterials (0x42BFE0);
    //                 cmd 35009            → OnSaveLayeredMaterials   (0x42C020).
    ON_COMMAND(35008, OnToggleLayeredMaterials)
    ON_COMMAND(35009, OnSaveLayeredMaterials)

    // PATCH menu — Patch_BrushToMesh primitives + Patch_AdjustSelected grid edits.
    ON_COMMAND(32856, OnCurveSimplepatchmesh)  // Curve→Simple Patch Mesh (density dialog, 0x429a20)
    ON_COMMAND(32939, OnCurveSimpleterrainpatch)// Curve→Simple Terrain Patch (density dialog)
    ON_COMMAND(32859, OnCurvePatchtube)        // Patch→Primitives→Cylinder
    ON_COMMAND(32860, OnCurvePatchcone)        // Patch→Primitives→Cone
    ON_COMMAND(32861, OnCurvePatchendcap)      // Patch→Primitives→End Cap
    ON_COMMAND(32862, OnCurvePatchbevel)       // Patch→Primitives→Bevel
    ON_COMMAND(32891, OnCurvePatchsquare)      // Patch→Primitives→Square Cylinder
    ON_COMMAND(32920, OnCurveSquareBevel)      // Patch→Primitives→Square Bevel
    ON_COMMAND(32921, OnCurveSquareEndcap)     // Patch→Primitives→Square End Cap
    ON_COMMAND(32874, OnCurveInsertcolumn)     // Patch→Insert→Insert (2) Columns
    ON_COMMAND(32873, OnCurveInsertAddcolumn)  // Patch→Insert→Add (2) Columns
    ON_COMMAND(32876, OnCurveInsertrow)        // Patch→Insert→Insert (2) Rows
    ON_COMMAND(32875, OnCurveInsertAddrow)     // Patch→Insert→Add (2) Rows
    ON_COMMAND(32877, OnCurveDeleteFirstcolumn)// Patch→Delete→First (2) Columns
    ON_COMMAND(32878, OnCurveDeleteLastcolumn) // Patch→Delete→Last (2) Columns
    ON_COMMAND(32879, OnCurveDeleteFirstrow)   // Patch→Delete→First (2) Rows
    ON_COMMAND(32880, OnCurveDeleteLastrow)    // Patch→Delete→Last (2) Rows
    ON_COMMAND(32881, OnCurveNegative)         // Patch→Negative (vertical flip / invert)
    ON_COMMAND(32906, OnCurveMatrixTranspose)  // Patch→Matrix→Transpose
    ON_COMMAND(32890, OnPatchNaturalize)       // Patch→Naturalize
    ON_COMMAND(33101, OnSelectionInvert)       // Selection→Invert (Ctrl+I)
    ON_COMMAND(32889, OnCurveRedisperseCols)   // Patch→Redisperse→Columns
    ON_COMMAND(32888, OnCurveRedisperseRows)   // Patch→Redisperse→Rows
    ON_COMMAND(32899, OnCurveNegativeTextureX) // Patch→Negative Texture X
    ON_COMMAND(32903, OnCurveNegativeTextureY) // Patch→Negative Texture Y
    // The five dead patch commands from the U7 backfill (Sphere 32922 has NO handler in
    // the binary either — see RADIANT_MISSING_FUNCTIONS.md).
    ON_COMMAND(32883, OnCurvePatchdensetube)     // Patch→Primitives→Dense Cylinder      (0x42AB90)
    ON_COMMAND(32884, OnCurvePatchverydensetube) // Patch→Primitives→Very Dense Cylinder (0x42AC40)
    ON_COMMAND(32905, OnCurveCyclecap)           // Patch→Cycle Cap Texture (Shift+Ctrl+N)(0x42B1A0)
    ON_COMMAND(33153, OnAddTerrainRowColumn)     // Patch→Insert→Add Terrain Row/Column   (0x42B080)

    // The remaining CMainFrame singles.
    ON_COMMAND(206,   OnDeleteExportables)       // Misc→Delete exportables               (0x424E30)
    ON_COMMAND(32790, OnHelpCommandlist)         // Help→Command list...                  (0x426E00)
    ON_COMMAND(1085,  OnLinkKeepSelection)       // Shift+O LinkSelectionToggle           (0x423EE0)
    ON_COMMAND(32776, ToggleCamera)              // camera-update preview flip            (0x423A50)
    ON_COMMAND(32779, OnPopupRenderMethod)       // HandlePopup(IDR_POPUP_RENDER_METH)    (0x4263E0)
    ON_COMMAND(32780, OnPopupSelection)          // HandlePopup(IDR_POPUP_SELECTION)      (0x4263F0)
    ON_COMMAND(32782, OnViewCameraupdate)        // View→Camera update                    (0x426450)
    ON_COMMAND(32863, OnCurvePatchinvertedendcap)// Primitives→Inverted End Cap (EMPTY)   (0x42A500)
    ON_COMMAND(32864, OnCurvePatchinvertedbevel) // Primitives→Inverted Bevel   (EMPTY)   (0x42A4F0)
    ON_COMMAND(32867, OnCurveInsertrowSingle)    // Ctrl+Num+      IncPatchRow            (0x42A5B0)
    ON_COMMAND(32868, OnCurveInsertcolumnSingle) // Shift+Ctrl+Num+ IncPatchColumn        (0x42A560)
    ON_COMMAND(32869, OnCurveDeleterowSingle)    // Ctrl+Num-      DecPatchRow            (0x42A650)
    ON_COMMAND(32870, OnCurveDeletecolumnSingle) // Shift+Ctrl+Num- DecPatchColumn        (0x42A600)
    ON_COMMAND(32882, OnPatchBend)               // patch bend-mode toggle                (0x42A950)
    ON_COMMAND(32925, OnHideUnselected2)         // Shift+Alt+Ctrl+H HideByClassname      (0x42B6B0)
    ON_COMMAND(32978, OnMiscBenchmark)           // Misc→Benchmark (EMPTY)                (0x424B70)
    ON_COMMAND(33089, OnPatchTab)                // Tab  Patch TAB                        (0x42A9E0)
    ON_COMMAND(33090, OnPatchEnter)              // Patch ENTER (EMPTY)                   (0x42A9D0)
    ON_COMMAND(33091, OnGotoPos_unk)             // Ctrl+G SelectSnapPointsToGrid         (0x42AE90)
    ON_COMMAND(33165, OnVertexSelectUp)          // Ctrl+Up   Vertex Select Up            (0x426880)
    ON_COMMAND(33166, OnVertexSelectDown)        // Ctrl+Down Vertex Select Down          (0x4268A0)
    ON_COMMAND(33167, OnVertexSelectRight)       // (no UI path in the binary)            (0x4268C0)
    ON_COMMAND(33168, OnVertexSelectLeft)        // (no UI path in the binary)            (0x4268E0)
    ON_COMMAND(33170, OnRedistPatchPoints)       // Shift+F RedisperseVertices            (0x42A270)
    ON_COMMAND(33179, OnTurnTerrainEdges)        // Alt+F2  AutoEdgeTurn                  (0x4294E0)
    ON_COMMAND(33213, OnDropPatchVertices)       // Shift+Alt+Ctrl+D DropVertices         (0x42AE00)
    ON_COMMAND(36110, OnSelectTargettedEntity)   // Ctrl+E  SelectTargettedEntities       (0x425560)
    ON_COMMAND(57602, OnFileClose)               // File→Close        (EMPTY)             (0x423A70)
    ON_COMMAND(57607, OnFilePrint)               // File→Print        (EMPTY)             (0x423B60)
    ON_COMMAND(57609, OnFilePrintPreview)        // File→Print Preview(EMPTY)             (0x423B70)

    // PATCH/CURVE OPERATION CLUSTER (Cap / Thicken / Weld / Split) — cores in pmesh.cpp,
    // ids from the CMainFrame AFX_MSGMAP.
    ON_COMMAND(32885, OnCurveCap)              // Curve→Cap                (Patch_CapCurrent)
    ON_COMMAND(35040, OnPatchCap)              // Patch→Cap (naturalize)   (0x42AE50)
    ON_COMMAND(32904, OnCurveThicken)          // Curve→Thicken            (Patch_Thicken + CDialogThick)
    ON_COMMAND(33158, OnSplitPatch)            // Curve→Split              (SplitPatch)
    // TERRAIN ROW/COLUMN CLUSTER — cores in pmesh.cpp.
    ON_COMMAND(33192, ExtrudeTerrainRow2)      // Terrain→Extrude Row/Col  (ExtrudeTerrainRow)
    ON_COMMAND(33154, OnRemoveTerrainRowColumn)// Terrain→Remove Row/Col   (RemoveTerrainRowCol)
    ON_COMMAND(35041, OnCurveToTerrain)        // Curve→Terrain            (PMESH_07_Width)
    ON_COMMAND(36102, OnFaceToTerrain)         // Face→Terrain             (PMESH_58)
    ON_COMMAND(33021, OnSelectionConnect)      // Selection→Connect        (WeldMesh/ConnectEntities_R)

    // VIEW->SHOW overlay toggles (d_xyShowFlags).
    ON_COMMAND(33971, OnSelectNames)           // View→Show→Names         (d_xyShowFlags ^ 0x8)
    ON_COMMAND(33972, OnSelectAngles)          // View→Show→Angles        (d_xyShowFlags ^ 0x2)
    ON_COMMAND(33973, OnSelectBlocks)          // View→Show→Blocks        (d_xyShowFlags ^ 0x10)
    ON_COMMAND(33974, OnSelectConnections)     // View→Show→Connections   (d_xyShowFlags ^ 0x4)
    ON_COMMAND(33975, OnSelectCoordinates)     // View→Show→Coordinates   (d_xyShowFlags ^ 0x20)
    ON_COMMAND(36127, OnSelectReverseFilter)   // View→Show→Reverse Filter(d_xyShowFlags ^ 0x40)

    // SCRIPT-GROUP / FIXED-SIZE LIGHT keyboard commands (cores in brush.cpp + pmesh.cpp).
    ON_COMMAND(33151, OnDisassociateEntities)  // DisassociateEntities (Alt+Ctrl+G)
    ON_COMMAND(33152, OnSelectedAssociated)    // SelectedAssociated (Ctrl+X)
    // ScriptGroup colour-dialog commands, ids from the CMainFrame AFX_MSGMAP.  9/10 are the
    // dialog's Add-Color / Disassociate control ids (they bubble to the frame); 200 is the menu
    // item and 33150 the accelerator - two distinct thunks that both call AssociateEntities.
    ON_COMMAND(9,     OnScriptGroup_01)          // dialog "Add Color"  → AddColorToSelection (0x4264A0)
    ON_COMMAND(10,    OnScriptGroup_Disassociate) // dialog "Disassociate" → TriggerNumber    (0x426460)
    ON_COMMAND(200,   OnScriptGroup)             // "Script group" menu item                  (0x424E20)
    ON_COMMAND(33150, OnAssociateEntities)       // Associate Entities accel (Shift+G)        (0x428E00)
    ON_COMMAND(33145, OnLightShiftUp)          // light value ×1.1 (Alt+])
    ON_COMMAND(33146, OnLightShiftDown)        // light value ×0.9 (Alt+[)
    ON_COMMAND(33176, OnCyclinderHeightUp)     // cylinder height ×1.1
    ON_COMMAND(33177, OnCyclinderHeightDown)   // cylinder height ×0.9
    ON_COMMAND(33055, OnCameraUp)              // raise 3D camera +32 ('D')                  (0x426900)
    ON_COMMAND(33056, OnCameraDown)            // lower 3D camera -32 ('C')                  (0x426680)
    ON_COMMAND(33061, OnCameraAngleUp)         // pitch 3D camera +22.5 ('A')                (0x4265d0)
    ON_COMMAND(33062, OnCameraAngleDown)       // pitch 3D camera -22.5 ('Z')                (0x426590)
    ON_COMMAND(33147, OnOverBrightShiftUp)     // overbrightShift -0.05 + Patch_Subdivide(-1) (Shift+])
    ON_COMMAND(33148, OnOverBrightShiftDown)   // overbrightShift +0.05 + Patch_Subdivide(+1) (Shift+[)

    // UI COMMAND-WIRING batch - thunks whose cores are already ported.
    // Camera fly keys (arrows / , .).
    ON_COMMAND(33057, OnCameraLeft)            // strafe-yaw left  (Left)        (0x426720)
    ON_COMMAND(33058, OnCameraRight)           // strafe-yaw right (Right)       (0x426770)
    ON_COMMAND(33059, OnCameraForward)         // move forward     (Up)          (0x4266B0)
    ON_COMMAND(33060, OnCameraBack)            // move back        (Down)        (0x426610)
    ON_COMMAND(33063, OnCameraStrafeleft)      // strafe left      (',')         (0x4267C0)
    ON_COMMAND(33064, OnCameraStraferight)     // strafe right     ('.')         (0x426820)
    ON_COMMAND(33065, OnGridToggle)            // toggle grid      ('0')         (0x426930)
    // Grid size / texture step hotkeys.
    ON_COMMAND(33083, OnGridNext)              // grid size up   (])            (0x4289A0)
    ON_COMMAND(33084, OnGridPrev)              // grid size down ([)            (0x4289D0)
    ON_COMMAND(33072, OnSelectionTextureSnapDec) // tex step dec (Shift+KP-)    (0x4287B0)
    ON_COMMAND(33073, OnSelectionTextureSnapInc) // tex step inc (Shift+KP+)    (0x428830)
    ON_COMMAND(33074, OnSelectionTextureFitUnk)  // texture fit  (Ctrl+F)       (0x4287D0)
    ON_COMMAND(33234, OnTextureFitAll)           // texture fit all             (0x428800)
    ON_COMMAND(33075, OnTexRotateClockwise)      // tex rotate CW  (Ctrl+Left)  (0x428850)
    ON_COMMAND(33076, OnTexRotateCounterCW)      // tex rotate CCW (Ctrl+Right) (0x428860)
    ON_COMMAND(33079, OnTexShiftLeft)            // tex shift left              (0x4288E0)
    ON_COMMAND(33080, OnTexShiftRight)           // tex shift right             (0x428910)
    ON_COMMAND(33081, OnTexShiftUp)              // tex shift up                (0x428870)
    ON_COMMAND(33082, OnTexShiftDown)            // tex shift down              (0x4288A0)
    // Z-view zoom + cubic-clip zoom.
    ON_COMMAND(32998, OnViewZ100)              // Z 100% (empty stub)           (0x424740)
    ON_COMMAND(32999, OnViewZzoomin)           // Z zoom in  (Ctrl+Del)         (0x424A00)
    ON_COMMAND(33000, OnViewZzoomout)          // Z zoom out (Ctrl+Ins)         (0x424A40)
    ON_COMMAND(32819, OnViewCubeout)           // cubic-clip zoom out (Ctrl+[)  (0x428F50)
    ON_COMMAND(32820, OnViewCubein)            // cubic-clip zoom in  (Ctrl+])  (0x428F10)
    // View layout / toggle / next.
    ON_COMMAND(32772, OnViewXy)                // View→Layout XY                (0x424710)
    ON_COMMAND(32774, OnViewYz)                // View→Layout YZ                (0x423FB0)
    ON_COMMAND(32773, OnViewXz)                // View→Layout XZ                (0x424A80)
    ON_COMMAND(32797, OnToggleviewYz)          // View→Toggle YZ (empty stub)   (0x427230)
    ON_COMMAND(32798, OnToggleviewXz)          // View→Toggle XZ (empty stub)   (0x427220)
    // View→Toggle→Console / Camera / Z / XY (Top) — show-hide the docked children.
    ON_COMMAND(33068, OnToggleconsole)         // View→Toggle→Console View      (0x426A90)
    ON_COMMAND(33069, OnTogglecamera)          // View→Toggle→Camera View       (0x426A40)
    ON_COMMAND(33070, OnTogglez)               // View→Toggle→Z View            (0x426B30)
    ON_COMMAND(33071, OnToggleview)            // View→Toggle→XY (Top)          (0x426AE0)
    ON_COMMAND(32789, OnViewNextview)          // Next view (Ctrl+Tab)          (0x426DB0)
    ON_COMMAND(32830, OnToolbarMain)           // toolbar Main    (empty stub)  (0x4290F0)
    ON_COMMAND(32832, OnToolbarTexture)        // toolbar Texture (empty stub)  (0x429100)
    ON_COMMAND(33108, OnCenter2DOnCamera)      // center 2D on camera (Shift+C) (0x42A2D0)
    // Selection cycle / move / nudge.
    ON_COMMAND(33160, OnSelectNext)            // select next     (Shift+.)     (0x423C90)
    ON_COMMAND(33161, OnSelectPrev)            // select prev     (Shift+,)     (0x423CA0)
    ON_COMMAND(32829, OnSelectionMovedown)     // move selection down (KP-)     (0x429050)
    ON_COMMAND(32831, OnSelectionMoveup)       // move selection up   (KP+)     (0x4290B0)
    ON_COMMAND(32847, OnSelectionSelectNudgeleft)  // nudge left  (Alt+Left)    (0x429510)
    ON_COMMAND(32848, OnSelectionSelectNudgeright) // nudge right (Alt+Right)   (0x429530)
    ON_COMMAND(32849, OnSelectionSelectNudgeup)    // nudge up    (Alt+Up)      (0x429550)
    ON_COMMAND(32850, OnSelectionSelectNudgedown)  // nudge down  (Alt+Down)    (0x4294F0)
    // Splay / view-to-entity / link / distance / select-all-of-type.
    ON_COMMAND(33157, OnSplay)                 // Splay (Ctrl+W)                (0x4254F0)
    ON_COMMAND(33210, OnSetViewToEntity)       // set view to entity (F6)       (0x425540)
    ON_COMMAND(33211, OnLinkSelected)          // link selected (Ctrl+Shift+K)  (0x425500)
    ON_COMMAND(33178, OnDistanceBetweenEntities) // get distance (Alt+F1)       (0x4294D0)
    ON_COMMAND(33093, OnSelectAllOfType)       // select all of type (Shift+A)  (0x42B470)
    ON_COMMAND(33212, OnSelectAllOfTypeRecursive) // select all recurse (Alt+A) (0x42B4B0)
    // Hide / Show.
    ON_COMMAND(32923, OnHideSelected)          // hide selected   (H)           (0x42B6A0)
    ON_COMMAND(32934, OnHideUnselected)        // hide unselected (Alt+H)       (0x42B6C0)
    ON_COMMAND(32924, OnShowHidden)            // show hidden     (Shift+H)     (0x42B6D0)
    ON_COMMAND(33246, OnShowLastHidden)        // show last hidden (Ctrl+H)     (0x42B6E0)
    // Draw toggles + same-target(name).
    ON_COMMAND(33100, OnViewCrosshair)         // toggle crosshairs (Shift+X)   (0x42B690)
    ON_COMMAND(33103, OnSelectionNoOutline)    // no outline (J)                (0x425630)
    ON_COMMAND(33172, OnSelectionNoTint)       // no tint    (Shift+J)          (0x425650)
    ON_COMMAND(36121, OnSelectSameTargetname)  // same targetname (B)           (0x42ADA0)
    ON_COMMAND(36123, OnSelectSameTarget)      // same target (Ctrl+B)          (0x42ADD0)

    // ── File menu (Load / Save-Selected / Save-Region). ──
    ON_COMMAND(32844, OnFileImportmap)         // File→Load  (merge .map)       (0x429290)
    ON_COMMAND(32845, OnFileExportmap)         // File→Save Selected            (0x4293A0)
    ON_COMMAND(32827, OnFileSaveregion)        // File→Save Region              (0x429020)

    // ── Light-preview submenu (F8 workflow). ──
    ON_COMMAND(33950, OnEnableLightPreview)    // enable light preview          (0x4240C0)
    ON_COMMAND(36108, OnPreviewSun)            // preview sun as well           (0x424060)
    ON_COMMAND(33951, OnStartPreviewSelected)  // start preview selected        (0x424120)
    ON_COMMAND(33952, OnStopPreviewSelected)   // stop preview selected         (0x424170)
    ON_COMMAND(33953, OnClearPreviewList)      // clear preview list            (0x4241C0)
    ON_COMMAND(36122, OnPreviewAtMaxIntensity) // preview at max intensity      (0x425670)

    // ── Colors menu (each: DoColor(idx) → Sys_UpdateWindows). ids/handlers/palette indices
    // decoded from the CMainFrame AFX_MSGMAP + each thunk body (verified against the binary). ──
    ON_COMMAND(33013, OnTextureBackground)         // Texture Background...        DoColor(0)
    ON_COMMAND(33014, OnColorsXyBackground)        // Grid Background...           DoColor(1)
    ON_COMMAND(33020, OnColorsMinor)               // Grid Minor...                DoColor(2)
    ON_COMMAND(33019, OnColorsMajor)               // Grid Major...                DoColor(3)
    ON_COMMAND(209,   OnColorsCameraBack)          // Camera Background...         DoColor(4)
    ON_COMMAND(32803, OnColorsGridblock)           // Grid Block...                DoColor(7)
    ON_COMMAND(32799, OnColorsGridText)            // Grid Text...                 DoColor(8)
    ON_COMMAND(32800, OnColorsBrush)               // Default Brush...             DoColor(9)
    ON_COMMAND(32801, OnColorsSelectedbrush)       // Selected Brush...            DoColor(10)
    ON_COMMAND(33171, OnColorsSelectedbrushCamera) // Selected Brush Camera Tint.. DoColor(11)
    ON_COMMAND(32802, OnColorsClipper)             // Clipper...                   DoColor(12)
    ON_COMMAND(32804, OnColorsViewname)            // Active View Name...          DoColor(13)
    ON_COMMAND(33131, OnColorsDetailBrush)         // Detail Brush...              DoColor(14)
    ON_COMMAND(33149, OnColorsToggleDrawSurfs)     // Draw toggle surfs...         DoColor(15)
    ON_COMMAND(33137, OnColorsSelfaceCamera)       // Selected Face Camera Tint... DoColor(16)
    ON_COMMAND(189,   OnColorsFuncGroup)           // Func Group...                DoColor(17)
    ON_COMMAND(211,   OnColorsFuncCullGroup)       // Func Cullgroup...            DoColor(18)
    ON_COMMAND(195,   OnColorsWeaponclip)          // Weapon Clip Brush...         DoColor(19)
    ON_COMMAND(212,   OnColorsSizeInfo)            // Size Info...                 DoColor(20)
    ON_COMMAND(213,   OnColorsModel)               // Model...                     DoColor(21)
    ON_COMMAND(208,   OnColorsUnknown208)          // (no menu item; msg-map only) DoColor(22)
    ON_COMMAND(33194, OnColorsWireframe)           // Wireframe...                 DoColor(23)
    ON_COMMAND(35020, OnColorsFrozenLayers)        // Frozen Layers...             DoColor(24)
    ON_COMMAND(33036, OnMiscSelectentitycolor)     // Select Entity Color... (K)   (0x424C10)
    // Colors→Themes presets (whole-palette writes).
    ON_COMMAND(32805, OnThemeQ4)                   // Themes→QE4 Original          (0x427480)
    ON_COMMAND(32806, OnThemeQ3)                   // Themes→Q3Radiant Original    (0x427760)
    ON_COMMAND(32807, OnThemeBlackGreen)           // Themes→Black and Green       (0x427A40)
    ON_COMMAND(33143, OnThemeInverted)             // Themes→Inverted              (0x427D30)
    ON_COMMAND(210,   OnThemeGrey)                 // Themes→Gray                  (0x428040)

    ON_COMMAND(32854, OnDynamicLighting)           // Dynamic Lighting popup camera (0x429960)

    // ── ON_UPDATE_COMMAND_UI — the binary's 4 enable/grey handlers (0% ported before). ──
    ON_UPDATE_COMMAND_UI(32782, OnUpdateViewCameraupdate) // View→Camera update  (0x4264B0)
    ON_UPDATE_COMMAND_UI(57643, OnUpdateEditUndo)         // Edit→Undo grey      (0x428750)
    ON_UPDATE_COMMAND_UI(57644, OnUpdateEditRedo)         // Edit→Redo grey      (0x428790)
    ON_UPDATE_COMMAND_UI(32827, OnUpdateFileSaveregion)   // File→Save Region    (0x429030)
END_MESSAGE_MAP()

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

// Default editor colours/grid so the views are legible without a registry/prefs load
// (Phase 6). colours: [1] XY background, [2] grid minor, [3] grid major, [4] camera
// background, [8] grid text, [9] brushes, [10] selected, [13] active view name.
static void Radiant_SetDefaultGridState()
{
    g_qeglobals.d_showgrid = true;
    // CreateQEChildren 0x4219d1: d_gridsize = 5 (grid_sizes[5] == 16.0) is the startup default.
    g_qeglobals.d_gridsize = 5;

    // Default palette, verbatim from MFCCreate (0x499600): WHITE XY background, grey grid,
    // BLACK text/brushes.  XY_DrawGrid draws each grid colour as its own
    // R_AddCmdSetMaterialColor batch, so the dark lines survive on the white background
    SavedInfo_t &si = g_qeglobals.d_savedinfo;
    si.iSize     = 0x2C4;
    si.iTextMenu = 32993;
    si.d_picmip  = 2;
    static const float kColors[23][4] = {
        { 0.25f, 0.25f, 0.25f, 1.0f },   //  0
        { 1.00f, 1.00f, 1.00f, 1.0f },   //  1  XY background (WHITE)
        { 0.75f, 0.75f, 0.75f, 1.0f },   //  2  grid minor (light grey)
        { 0.50f, 0.50f, 0.50f, 1.0f },   //  3  grid major (mid grey)
        { 0.25f, 0.25f, 0.25f, 1.0f },   //  4  camera background
        { 0.00f, 0.00f, 0.00f, 0.0f },   //  5  (unset by MFCCreate)
        { 0.00f, 0.00f, 0.00f, 0.0f },   //  6  (unset)
        { 0.00f, 0.00f, 1.00f, 1.0f },   //  7  block grid (blue)
        { 0.00f, 0.00f, 0.00f, 1.0f },   //  8  grid text (BLACK)
        { 0.00f, 0.00f, 0.00f, 1.0f },   //  9  brushes (BLACK)
        { 1.00f, 0.00f, 0.00f, 1.0f },   // 10  selected (red)
        { 1.00f, 0.25f, 0.25f, 0.25f },  // 11
        { 0.00f, 0.00f, 1.00f, 1.0f },   // 12  (blue)
        { 0.50f, 0.00f, 0.75f, 1.0f },   // 13  active view name (purple)
        { 0.00f, 0.60f, 0.00f, 1.0f },   // 14  (green)
        { 0.00f, 0.00f, 0.00f, 0.0f },   // 15  (unset)
        { 1.00f, 0.25f, 0.25f, 0.25f },  // 16
        { 0.75f, 0.75f, 0.75f, 1.0f },   // 17
        { 0.75f, 0.75f, 0.75f, 1.0f },   // 18
        { 0.50f, 0.60f, 0.00f, 1.0f },   // 19
        { 0.65f, 0.00f, 0.00f, 1.0f },   // 20
        { 0.85f, 0.00f, 0.85f, 1.0f },   // 21
        { 0.80f, 0.60f, 0.00f, 1.0f },   // 22
    };
    for ( int i = 0; i < 23; ++i )
        for ( int c = 0; c < 4; ++c )
            si.colors[i][c] = kColors[i][c];
}

// KISAK: the binary's OnCreateClient (0x422480) builds nested CSplitterWnds - outer 2 rows
// (views 85% / console CEdit 15%, whose HWND becomes d_hwndEdit), inner 3 columns.  The
// renderer multi-window path is driven by HWND, not splitter panes, so the port reproduces
// that arrangement by hand: a bottom console strip, then Z | XY | (Cam over Tex).
struct EdLayout { CRect xy, cam, z, tex, ent, con; };
// Width (px) of the splitter gutters left between adjacent panes (the draggable bars).
static const int GAP = 4;

// Splitter bars: the panes are laid out by hand with GAP-wide gutters (client area covered by
// no child pane); the WM_SETCURSOR / WM_LBUTTON* / WM_MOUSEMOVE handlers drag these fractions
// (defaults = the binary's OnCreateClient splits).
static float s_fracConsole = 0.15f;  // console height / inner (views+console) height
static float s_fracZ       = 0.05f;  // Z-strip width / cx (IDA OnCreateClient col0 = 5%)
static float s_fracRight   = 0.25f;  // right-column width / cx (IDA OnCreateClient col2 = 25%)
static float s_fracCam     = 0.60f;  // camera height / views height (IDA split3 row0 = 60%)
static float s_fracEnt     = 0.20f;  // entity-inspector height / views height
// Gutter geometry (client coords) cached by Radiant_ComputeLayout for the hit-test.
static int s_barV1x, s_barV2x;            // vertical gutters: Z|XY, XY|right
static int s_barH1y, s_barH2y, s_barH3y;  // horizontal gutters: cam|ent, ent|tex, views|console
static int s_barViewsTop, s_barViewsBot;  // y-span of the vertical gutters
static int s_barRightL, s_barRightR;      // x-span of the cam|ent / ent|tex gutters
static int s_barCx, s_barInnerH, s_barTopInset, s_barTopH;  // drag-math references
static int s_dragBar = 0;                 // gutter currently being dragged (0 = none)
static inline float Clampf( float v, float lo, float hi ) { return v < lo ? lo : ( v > hi ? hi : v ); }

struct RadiantSplitInfo
{
    int min;
    int cur;
};

static bool Radiant_LoadSplitInfo( const char *name, RadiantSplitInfo *out )
{
    long size = sizeof( *out );
    return LoadRegistryInfo( name, out, &size ) && size >= (long)sizeof( *out ) && out->cur > 0;
}

// CMainFrame::OnCreateClient 0x422480 seeds the three splitter trees with 85/15,
// 5/70/25, 60/40, then LoadRegistryInfo overrides Row_*/Col_* with persisted
// CSplitterWnd row/column info.  The port's panes are manual HWNDs, so translate the
// same saved current sizes into the fraction state that Radiant_ComputeLayout consumes.
static void Radiant_LoadSavedSplitterLayout()
{
    static bool s_loaded = false;
    if ( s_loaded )
        return;
    s_loaded = true;

    RadiantSplitInfo row0, row1;
    if ( Radiant_LoadSplitInfo( "Radiant::Split::Row_0", &row0 ) &&
         Radiant_LoadSplitInfo( "Radiant::Split::Row_1", &row1 ) )
    {
        int total = row0.cur + row1.cur;
        if ( total > 0 )
            s_fracConsole = Clampf( (float)row1.cur / (float)total, 0.01f, 0.80f );
    }

    RadiantSplitInfo col0, col1, col2;
    if ( Radiant_LoadSplitInfo( "Radiant::Split2::Col_0", &col0 ) &&
         Radiant_LoadSplitInfo( "Radiant::Split2::Col_1", &col1 ) &&
         Radiant_LoadSplitInfo( "Radiant::Split2::Col_2", &col2 ) )
    {
        int total = col0.cur + col1.cur + col2.cur;
        if ( total > 0 )
        {
            s_fracZ     = Clampf( (float)col0.cur / (float)total, 0.005f, 0.40f );
            s_fracRight = Clampf( (float)col2.cur / (float)total, 0.01f, 0.85f );
        }
    }

    RadiantSplitInfo split3row0, split3row1;
    if ( Radiant_LoadSplitInfo( "Radiant::Split3::Row_0", &split3row0 ) &&
         Radiant_LoadSplitInfo( "Radiant::Split3::Row_1", &split3row1 ) )
    {
        int total = split3row0.cur + split3row1.cur;
        if ( total > 0 )
            s_fracCam = Clampf( (float)split3row0.cur / (float)total, 0.05f, 0.95f );
    }
}

static void Radiant_RestoreMainWindowPlacement( CMainFrame *frame )
{
    WINDOWPLACEMENT wp;
    memset( &wp, 0, sizeof( wp ) );
    wp.length = sizeof( wp );
    long size = sizeof( wp );
    if ( LoadRegistryInfo( "Radiant::MainWindowPlace", &wp, &size ) && size >= (long)sizeof( wp ) )
    {
        wp.length = sizeof( wp );
        frame->SetWindowPlacement( &wp );
    }
}

// topInset reserves a strip at the top of the frame for the docked texture bar (the QE4
// views + console lay out below it).  The layout is computed for the reduced height and then
// shifted down by topInset.
static EdLayout Radiant_ComputeLayout( int cx, int cy, int topInset = 0 )
{
    Radiant_LoadSavedSplitterLayout();

    if ( topInset < 0 ) topInset = 0;
    if ( topInset > cy - 64 ) topInset = 0;   // never starve the views
    cy -= topInset;
    if ( cx < 64 ) cx = 64;
    if ( cy < 64 ) cy = 64;

    // Bottom console strip (the binary's m_wndSplit row 1, 15% of the frame). Clamp so a
    // small window keeps usable views above it and never collapses the console to 0.
    int conH = (int)( cy * s_fracConsole );
    if ( conH < 5 ) conH = 5;              // IDA SetRowInfo row1 min = 5
    if ( conH > cy - 50 ) conH = cy - 50;  // IDA row0 min = 50
    if ( conH < 1 ) conH = 1;
    int topH = cy - conH;                   // the views occupy [0, topH)
    if ( topH < 64 ) { topH = cy; conH = 0; }

    // Binary OnCreateClient 0x422480 (nView 0, after the left/right pane-ID swap): Z strip 5%
    // left, XY grid 70% middle, right column 25% = Camera over texture browser.  (The binary
    // hides the Entity window in combined view; here it floats - see below.)
    int zW = (int)( cx * s_fracZ );         // Z strip (left)
    if ( zW < 10 ) zW = 10;                 // IDA split2 col0 min = 10
    int rW = (int)( cx * s_fracRight );     // right column (camera + inspector + textures)
    if ( rW < 25 ) rW = 25;                 // IDA split2 col2 min = 25
    // Cap only on the XY grid's MINIMUM width: the binary's CSplitterWnd has no maximum for
    // the right column, so let it grow to whatever the drag handler allows (0.85).
    const int xyMinW = 100;                 // IDA split2 col1 min = 100
    if ( rW > cx - zW - xyMinW ) rW = cx - zW - xyMinW;
    if ( rW < 25 ) rW = 25;                 // re-assert the min (tiny frames)
    int xyW = cx - zW - rW;                 // XY grid (the big middle pane)
    if ( xyW < 40 ) xyW = 40;               // absolute safety floor

    const int zRight  = zW;
    const int xyRight = zW + xyW;

    // Right column = Camera over Texture browser.  The Entity inspector FLOATS (toggled by
    // N/O/T/F), so s_fracEnt is unused.
    int rCamH = (int)( topH * s_fracCam );  // Camera on top, Texture browser below
    if ( rCamH < 15 ) rCamH = 15;           // IDA split3 min = 15/15
    if ( rCamH > topH - 15 ) rCamH = topH - 15;
    const int camBottom = rCamH;

    EdLayout L;
    L.z.SetRect  ( 0,              0,               zRight,  topH      );
    L.xy.SetRect ( zRight  + GAP,  0,               xyRight, topH      );
    L.cam.SetRect( xyRight + GAP,  0,               cx,      camBottom );
    L.tex.SetRect( xyRight + GAP,  camBottom + GAP, cx,      topH      );
    L.ent.SetRect( 0, 0, 0, 0 );            // entity inspector floats (toggled) — not docked
    L.con.SetRect( 0,              topH + GAP,      cx,      cy        );   // full-width console
    if ( topInset > 0 )
    {
        L.xy.OffsetRect ( 0, topInset );  L.cam.OffsetRect( 0, topInset );
        L.z.OffsetRect  ( 0, topInset );  L.tex.OffsetRect( 0, topInset );
        L.ent.OffsetRect( 0, topInset );  L.con.OffsetRect( 0, topInset );
    }
    // Cache gutter geometry (client coords) for the splitter-bar hit-test (CMainFrame mouse
    // handlers).  cy here is the inner (views+console) height after the topInset subtraction.
    s_barCx = cx; s_barInnerH = cy; s_barTopInset = topInset; s_barTopH = topH;
    s_barV1x = L.z.right;   s_barV2x = L.xy.right;
    s_barH1y = L.cam.bottom; s_barH2y = -10000; s_barH3y = L.con.top;  // H1 = cam|tex; ent floats (no H2)
    s_barViewsTop = L.xy.top; s_barViewsBot = L.xy.bottom;
    s_barRightL = L.cam.left; s_barRightR = L.cam.right;
    return L;
}

// Set once init is fully up (device created). OnPaint only renders when true.
bool g_radiantFirstLightRendererReady = false;

// Engine subsystem bring-up (once, BEFORE the windows/renderer): the slice of InitInstance /
// Com_Init that precedes window creation.  NOT the renderer - R_BeginRegistrationInternal owns
// R_InitRenderCommands + Com_InitHunkMemory + R_InitEditor + the per-window attaches.
static void Radiant_EngineInit()
{
    Radiant_FL_Log( "EngineInit: Com_InitThreadData(MAIN)" ); Com_InitThreadData( THREAD_CONTEXT_MAIN );
    Radiant_FL_Log( "EngineInit: Sys_InitializeCriticalSections" ); Sys_InitializeCriticalSections();
    Radiant_FL_Log( "EngineInit: track_init" );          track_init();
    Radiant_FL_Log( "EngineInit: SL_Init" );             SL_Init();
    Radiant_FL_Log( "EngineInit: Cbuf_Init" );           Cbuf_Init();
    Radiant_FL_Log( "EngineInit: Cmd_Init" );            Cmd_Init();
    Radiant_FL_Log( "EngineInit: Dvar_Init" );           Dvar_Init();
    Radiant_FL_Log( "EngineInit: Radiant_RegisterGroupCDvars" ); Radiant_RegisterGroupCDvars();

    // gfxCfg is zero-initialised in the radiant build (the client's SetupGfxConfig is not
    // compiled in). R_InitRenderCommands (inside R_BeginRegistrationInternal) sizes its
    // command/scene buffers by gfxCfg.maxClientViews — 0 would allocate nothing and fault.
    Radiant_FL_Log( "EngineInit: SetupGfxConfig" );
    gfxCfg.maxClientViews     = 1;
    gfxCfg.entCount           = 2208;
    gfxCfg.entnumNone         = ENTITYNUM_NONE;
    gfxCfg.entnumOrdinaryEnd  = ENTITYNUM_WORLD;
    gfxCfg.threadContextCount = THREAD_CONTEXT_COUNT;
    gfxCfg.critSectCount      = CRITSECT_COUNT;

    // FS before the hunk, as in the binary (Hunk_Init runs later inside
    // R_BeginRegistrationInternal - a second Com_InitHunkMemory here trips its !s_hunkData
    // assert).  useFastFile is already registered, so FS's IsFastFileLoad reads a valid dvar.
    Radiant_FL_Log( "EngineInit: FS_InitFilesystem" );   FS_InitFilesystem();
    Radiant_FL_Log( "EngineInit: done" );
}

// ── Create the five render windows + the entity placeholder, store all HWNDs ──────
static bool Radiant_CreateRenderWindows( CMainFrame *frame, const EdLayout &L )
{
    // XY (window 0 in the layout, but Camera is the device window — see below).
    frame->m_pXYWnd = new CXYWnd();
    if ( !frame->m_pXYWnd->Create( NULL, NULL, WS_CHILD | WS_VISIBLE, L.xy, frame, AFX_IDW_PANE_FIRST ) )
        return false;
    frame->m_pActiveXY           = frame->m_pXYWnd;
    frame->m_pXYWnd->m_nViewType = ED_VIEW_XY;
    g_qeglobals.d_hwndXY         = frame->m_pXYWnd->GetSafeHwnd();

    frame->m_pCamWnd = new CCamWnd();
    if ( !frame->m_pCamWnd->Create( NULL, NULL, WS_CHILD | WS_VISIBLE, L.cam, frame, AFX_IDW_PANE_FIRST + 1 ) )
        return false;
    g_qeglobals.d_hwndCamera = frame->m_pCamWnd->GetSafeHwnd();

    frame->m_pZWnd = new CZWnd();
    if ( !frame->m_pZWnd->Create( NULL, NULL, WS_CHILD | WS_VISIBLE, L.z, frame, AFX_IDW_PANE_FIRST + 2 ) )
        return false;
    g_qeglobals.d_hwndZ = frame->m_pZWnd->GetSafeHwnd();

    // Texture browser (right pane) — the real CTexWnd material palette.
    frame->m_pTexWnd = new CTexWnd();
    if ( !frame->m_pTexWnd->Create( NULL, NULL, WS_CHILD | WS_VISIBLE, L.tex, frame, AFX_IDW_PANE_FIRST + 3 ) )
        return false;
    g_qeglobals.d_hwndTexture = frame->m_pTexWnd->GetSafeHwnd();

    // LayeredMaterialWnd content — a hidden minimal render shell (the layered-material
    // browser is Phase 6). R_BeginRegistrationInternal asserts + attaches it, so it needs
    // a valid sized HWND; keep it off-screen/hidden (WS_CHILD without WS_VISIBLE).
    frame->m_pLayMat = new CEdBlankPane();
    if ( !frame->m_pLayMat->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 64, 64 ), frame, AFX_IDW_PANE_FIRST + 4 ) )
        return false;
    lyrMtlWndGlob.layerList = frame->m_pLayMat->GetSafeHwnd();

    // Entity inspector (d_hwndEntity): CEntityWnd_WEnt_Create 0x496850 makes this a FLOATING
    // owned popup ("QENT": caption + resize frame + sysmenu, owner = the main frame), HIDDEN
    // by default and summoned by N/O/T/F.  MFC's CWnd::CreateEx aborts building a control host
    // directly as WS_POPUP, so create it as a hidden CHILD and convert it below.
    frame->m_pEntWnd = new CEntityWnd();
    if ( !frame->m_pEntWnd->Create( NULL, NULL, WS_CHILD, L.ent, frame, AFX_IDW_PANE_FIRST + 5 ) )
        return false;
    g_qeglobals.d_hwndEntity = frame->m_pEntWnd->GetSafeHwnd();
    {
        HWND he = frame->m_pEntWnd->GetSafeHwnd();
        ::SetParent( he, NULL );                                   // child → top-level
        frame->m_pEntWnd->ModifyStyle( WS_CHILD, WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU );
        ::SetWindowLongA( he, GWL_HWNDPARENT, (LONG)(intptr_t)frame->GetSafeHwnd() ); // owned by main frame
        ::SetWindowTextA( he, "Entity" );
        WINDOWPLACEMENT wp;
        memset( &wp, 0, sizeof( wp ) );
        wp.length = sizeof( wp );
        long size = sizeof( wp );
        if ( LoadRegistryInfo( "EntityWindowPlace", &wp, &size ) && size >= (long)sizeof( wp ) )
        {
            wp.length = sizeof( wp );
            ::SetWindowPlacement( he, &wp );             // CEntityWnd_CreateEntityWindow 0x496920
            ::ShowWindow( he, SW_HIDE );
        }
        else
        {
            ::SetWindowPos( he, NULL, 160, 120, 360, 520,
                            SWP_NOZORDER | SWP_FRAMECHANGED | SWP_HIDEWINDOW );
        }
    }

    // Console pane (d_hwndEdit).  The binary's is a CEditWnd created as m_wndSplit's bottom
    // pane; CEditWnd::PreCreateWindow (0x40F5B0) gives it style 0x50200084 - note NOT
    // ES_READONLY, which is why console_print's EM_REPLACESEL works.  Com_Printf / Sys_Printf
    // route here through console_print (win_qe3.cpp).
    {
        const DWORD kConsoleStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                    ES_MULTILINE | ES_AUTOHSCROLL;   // 0x50200084
        if ( !frame->m_wndConsole.CreateEx( WS_EX_CLIENTEDGE, _T("EDIT"), NULL,
                  kConsoleStyle, L.con, frame, AFX_IDW_PANE_FIRST + 6 ) )
            return false;
        g_qeglobals.d_hwndEdit = frame->m_wndConsole.GetSafeHwnd();
        // The binary sets the console font to DEFAULT_GUI_FONT (OnCreateClient tail
        // 0x4232B0: SendMessage(g_pEdit, WM_SETFONT, GetStockObject(17), 1)).
        ::SendMessageA( g_qeglobals.d_hwndEdit, WM_SETFONT,
                        (WPARAM)GetStockObject( DEFAULT_GUI_FONT ), TRUE );
        // Install console_print as the editor-log sink (the binary's console_stuff =
        // &console_print): now Com_PrintMessage / Com_PrintError / R_Warn append to
        // the console pane.  Sys_Printf already calls console_print directly.
        SetConsoleHandler( console_print );
    }

    Radiant_FL_Log( "windows: XY=%p Cam=%p Z=%p Tex=%p LayMat=%p Ent=%p Console=%p",
        (void *)g_qeglobals.d_hwndXY, (void *)g_qeglobals.d_hwndCamera,
        (void *)g_qeglobals.d_hwndZ, (void *)g_qeglobals.d_hwndTexture,
        (void *)lyrMtlWndGlob.layerList, (void *)g_qeglobals.d_hwndEntity,
        (void *)g_qeglobals.d_hwndEdit );
    return true;
}

// ── Status bar (six panes; brush/entity counts land in pane 2, matching the binary) ─
static UINT s_statusIndicators[6] =
{
    ID_SEPARATOR, ID_SEPARATOR, ID_SEPARATOR, ID_SEPARATOR, ID_SEPARATOR, ID_SEPARATOR,
};

static void Radiant_CreateStatusBar( CMainFrame *frame )
{
    if ( !frame->m_wndStatusBar.Create( frame ) ||
         !frame->m_wndStatusBar.SetIndicators( s_statusIndicators, 6 ) )
    {
        Radiant_FL_Log( "status bar create FAILED" );
        return;
    }
    // Pane 0 stretches (general status); 1..5 are fixed-width info panes.
    frame->m_wndStatusBar.SetPaneInfo( 0, ID_SEPARATOR, SBPS_STRETCH, 0 );
    for ( int i = 1; i < 6; ++i )
        frame->m_wndStatusBar.SetPaneInfo( i, ID_SEPARATOR, SBPS_NORMAL, 150 );
    frame->m_wndStatusBar.SetPaneText( 0, "CoD4Radiant" );
}

// Status-bar sink (the real MainFrm_SetStatusText; replaces the engine_stubs no-op).
void MainFrm_SetStatusText( int pane, const char *text )
{
    if ( !g_pParentWnd || !text )
        return;
    if ( g_pParentWnd->m_wndStatusBar.GetSafeHwnd() && pane >= 0 && pane < 6 )
        g_pParentWnd->m_wndStatusBar.SetPaneText( pane, text );
}

// ── Map open (File→Open + the cmdline startup share this) ────────────────────────
static char s_currentMapPath[MAX_PATH] = "";

static void Radiant_CenterXYOnMap( CXYWnd *xy )
{
    if ( !xy ) return;
    float mins[3] = {  1e30f,  1e30f,  1e30f };
    float maxs[3] = { -1e30f, -1e30f, -1e30f };
    bool  any = false;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        brush_t *def = b->def;
        if ( !def ) continue;
        for ( int i = 0; i < 3; ++i )
        {
            if ( def->mins[i] < mins[i] ) mins[i] = def->mins[i];
            if ( def->maxs[i] > maxs[i] ) maxs[i] = def->maxs[i];
        }
        any = true;
    }
    if ( !any ) return;

    xy->m_vOrigin[0] = 0.5f * ( mins[0] + maxs[0] );
    xy->m_vOrigin[1] = 0.5f * ( mins[1] + maxs[1] );
    xy->m_vOrigin[2] = 0.5f * ( mins[2] + maxs[2] );

    int   w = xy->m_nWidth  > 0 ? xy->m_nWidth  : 1024;
    int   h = xy->m_nHeight > 0 ? xy->m_nHeight : 768;
    float ex = ( maxs[0] - mins[0] ) > 1.0f ? ( maxs[0] - mins[0] ) : 1.0f;
    float ey = ( maxs[1] - mins[1] ) > 1.0f ? ( maxs[1] - mins[1] ) : 1.0f;
    float sx = (float)w / ex, sy = (float)h / ey;
    float s  = ( sx < sy ? sx : sy ) * 0.9f;
    if ( s < 0.01f ) s = 0.01f;
    if ( s > 1.0f  ) s = 1.0f;
    xy->m_fScale = s;
}

static bool Radiant_PathLooksLikeMap( const char *p )
{
    if ( !p ) return false;
    size_t n = strlen( p );
    return n > 4 && _stricmp( p + n - 4, ".map" ) == 0;
}

// SetKeyValue (entity.cpp 0x483690).  Its side-effect chain is inert for "mapspath":
// Checkkey_Model fires only for model/angles/modelscale, Checkkey_Color only for
// _color/targetname.
extern void SetKeyValue( entity_s_def *e, const char *key, const char *value );

// LayerdMatWnd (layeredmaterials.cpp 0x416D40) - load the library named by the project
// entity's "layeredmaterials" epair into lyrMtlGlob.Layers[] (the binary calls it from
// QE_LoadProject right after Load_Textures).
extern signed int LayerdMatWnd();

// Project "mapspath" seed for prefab loading on a GUI map-open.  A stock .map references its
// geometry + prefabs via misc_prefab "model" keys, which Prefab_Load resolves as
// <project mapspath>\<name>.  Seed mapspath = the opened map's own directory (where the stock
// prefab tree lives), matching the real editor's project mapspath (= the map_source dir).
static void Radiant_SetProjectMapsPath( const char *mapPath )
{
    if ( !mapPath )
        return;

    // Directory portion of mapPath (strip the trailing "\<file>.map").
    char dir[MAX_PATH];
    _snprintf( dir, sizeof( dir ), "%s", mapPath );
    dir[sizeof( dir ) - 1] = '\0';
    char *slash = strrchr( dir, '\\' );
    char *fslash = strrchr( dir, '/' );
    if ( fslash && ( !slash || fslash > slash ) )
        slash = fslash;
    if ( !slash )
        return;                       // no directory component — leave mapspath as is
    *slash = '\0';

    // Allocate the project entity DEF on first use (140-byte entity_s_def, brush-list
    // sentinels self-linked — matching Entity_Create's LABEL_10 def init).
    entity_s_def *proj = g_qeglobals.d_project_entity;
    if ( !proj )
    {
        proj = (entity_s_def *)operator new( 0x8Cu );
        memset( proj, 0, 0x8Cu );
        proj->brushes.prev         = (selbrush_t *)&proj->def;
        proj->def = (entity_s *)&proj->def;
        g_qeglobals.d_project_entity = proj;
    }

    SetKeyValue( proj, "mapspath", dir );

    // The binary's QE_LoadProject parses cod4.prj's
    //   "layeredmaterials" "cod4_layered_material_library.txt"
    // epair then calls LayerdMatWnd() to load that library.  Seed the same epair and run the
    // loader.  LayerdMatWnd's LoadFile is a plain fopen relative to the editor CWD (no FS
    // search paths), so the file must sit beside the exe; absent, the library stays empty.
    SetKeyValue( proj, "layeredmaterials", "cod4_layered_material_library.txt" );
    LayerdMatWnd();
}

// Startup project load: resolve cod4.prj and run the full QE_LoadProject_ParseFile parse
// (populate d_project_entity with every epair, resolve the search-path keys, register
// fs_basepath/basegame/game).  Tier order is CreateQEChildren's: LastProject, then the
// install cod4.prj (the cmdline .map is handled separately).
static bool g_projectLoadedAtStartup = false;
static bool Radiant_LoadProjectAtStartup()
{
    extern signed int QE_LoadProject_ParseFile( const char *path );   // qe3.cpp 0x48BAB0
    extern const char *Dvar_GetString( const char *dvarName );        // qcommon (fs_basepath)

    // Tier 1 (binary CreateQEChildren): the registry "Prefs"/"LastProject" (Set Startup Project).
    if ( g_PrefsDlg && g_PrefsDlg->m_bLoadLast )
    {
        CString last = AfxGetApp()->GetProfileString( "Prefs", "LastProject", "" );
        if ( !last.IsEmpty() && QE_LoadProject_ParseFile( (const char *)last ) )
        {
            g_projectLoadedAtStartup = true;
            Radiant_FL_Log( "QE_LoadProject: loaded LastProject %s", (const char *)last );
            return true;
        }
    }

    // Tier 2: the stock cod4.prj beside the editor (bin\cod4.prj) — resolve via fs_basepath
    // (the install root the engine derived), like the RadiantFilters.txt / materials/*.txt
    // resolvers.  Try the CWD first (faithful: LoadFileNoCrash's plain fopen), then
    // <fs_basepath>\bin\cod4.prj.
    if ( QE_LoadProject_ParseFile( "cod4.prj" ) )
    {
        g_projectLoadedAtStartup = true;
        Radiant_FL_Log( "QE_LoadProject: loaded cod4.prj (cwd)" );
        return true;
    }
    const char *base = Dvar_GetString( "fs_basepath" );
    if ( base && *base )
    {
        char prjPath[MAX_PATH];
        _snprintf( prjPath, sizeof( prjPath ), "%s\\bin\\cod4.prj", base );
        prjPath[sizeof( prjPath ) - 1] = 0;
        if ( QE_LoadProject_ParseFile( prjPath ) )
        {
            g_projectLoadedAtStartup = true;
            Radiant_FL_Log( "QE_LoadProject: loaded %s", prjPath );
            return true;
        }
    }

    Radiant_FL_Log( "QE_LoadProject: no .prj found — using default seed" );
    return false;
}

void Radiant_OpenMap( const char *path )
{
    if ( !Radiant_PathLooksLikeMap( path ) )
        return;
    Radiant_FL_Log( "OpenMap: %s", path );
    // Seed the project mapspath from the map's own directory so misc_prefab "model"
    // refs (the *_geo.map geometry + prefabs/) resolve in Prefab_Load on this load.
    Radiant_SetProjectMapsPath( path );
    Map_LoadFromFile( path );
    _snprintf( s_currentMapPath, sizeof( s_currentMapPath ), "%s", path );

    if ( g_pParentWnd )
    {
        Z_CenterOnMap();
        // NO Cam_CenterOnMap here: Map_LoadFromFile (0x486680) already placed camera.origin at
        // the info_player_start (else deathmatch, else origin) +60 Z and set camera.angles from
        // that entity, exactly as the binary does.  (Radiant_CenterXYOnMap / Z_CenterOnMap keep
        // the 2D views map-centered - a port convenience; the binary points the XY view at the
        // player start too, via the m_pXYWnd write in 0x486680.)
        // Title = "CoD4Radiant - <map>".
        char title[MAX_PATH + 32];
        _snprintf( title, sizeof( title ), "CoD4Radiant - %s", path );
        g_pParentWnd->SetWindowText( title );
        // The eclass list grows as the map's entity classes are registered (Eclass_ForName
        // during parse) — refresh the entity-window list so they appear.
        if ( g_pParentWnd->m_pEntWnd && g_pParentWnd->m_pEntWnd->GetSafeHwnd() )
        {
            g_pParentWnd->m_pEntWnd->RefreshClassList();
            Entity_UpdateSelection();          // show worldspawn's key/values initially
        }
    }
    QE_CountBrushesAndUpdateStatusBar();
    g_nUpdateBits = W_ALL;

    // KISAK operator switch: RADIANT_STARTUP_CAM="x y z pitch yaw roll" places the 3D camera
    // after the load and rebuilds its view basis (a cmdline map bypasses radiantapp.cpp's
    // RADIANT_STARTUP_MAP path, so the hook is repeated here).
    if ( const char *camStr = getenv( "RADIANT_STARTUP_CAM" ) )
    {
        if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
        {
            float x = 0, y = 0, z = 0, pi = 0, ya = 0, ro = 0;
            if ( sscanf( camStr, "%f %f %f %f %f %f", &x, &y, &z, &pi, &ya, &ro ) >= 3 )
            {
                g_pParentWnd->m_pCamWnd->camera.origin[0] = x;
                g_pParentWnd->m_pCamWnd->camera.origin[1] = y;
                g_pParentWnd->m_pCamWnd->camera.origin[2] = z;
                g_pParentWnd->m_pCamWnd->camera.angles[0] = pi;
                g_pParentWnd->m_pCamWnd->camera.angles[1] = ya;
                g_pParentWnd->m_pCamWnd->camera.angles[2] = ro;
                g_pParentWnd->m_pCamWnd->Cam_BuildMatrix();
                Radiant_FL_Log( "STARTUP_CAM: camera.origin=(%.1f %.1f %.1f) angles=(%.1f %.1f %.1f)",
                                x, y, z, pi, ya, ro );
            }
        }
    }

}

static void Radiant_CheckGridMenu( CMainFrame *frame );   // Grid menu radio-check (defined below)
static void Radiant_CheckMenu( CMainFrame *frame, UINT id, bool checked );   // (defined below)

int CMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    Radiant_FL_LogReset();
    AddVectoredExceptionHandler( 1, Radiant_FL_Veh );
    Radiant_FL_Log( "CMainFrame::OnCreate begin" );
    if ( CFrameWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;

    Radiant_RestoreMainWindowPlacement( this );

    g_pParentWnd          = this;
    g_qeglobals.d_hInstance = AfxGetInstanceHandle();
    g_qeglobals.d_hwndMain  = GetSafeHwnd();
    Radiant_SetDefaultGridState();

    // 1) Engine subsystems (threads / SL / dvars / FS) — before windows + renderer.
    Radiant_EngineInit();

    // 1a) Load the project file (cod4.prj) — the FULL .prj parse (binary QE_LoadProject
    //     0x48BAB0, called from CreateQEChildren).  This populates g_qeglobals.d_project_entity
    //     with ALL the project epairs (basepath/mapspath/entitypath/game/basegame/autosave/
    //     layeredmaterials), resolves the 4 search-path keys to full paths, and registers
    //     fs_basepath/basegame/game from them.  It SUBSUMES the old single-epair "mapspath"
    //     seed that Radiant_SetProjectMapsPath used to do at map-open time.  cod4.prj ships in
    //     <install>\bin\ next to the editor; the dev build resolves it via the fs_basepath the
    //     engine already derived (the install root).  If it is absent, QE_LoadProject_ParseFile
    //     returns 0 (faithful missing-.prj branch) and the map-open path re-seeds mapspath as
    //     before — the editor still starts.
    Radiant_LoadProjectAtStartup();

    // 1a-bis) Create the MRU (recent-files) menu structure (binary CMainFrame::OnCreate
    //     0x4210b0: d_lpMruMenu = CreateMruMenuDefault()).  The registry load + menu insertion
    //     happen after the menu is set (step 4c below), matching SetButtonMenuStates 0x420039.
    {
        extern LPMRUMENU *CreateMruMenuDefault();   // qe3.cpp 0x48A150
        g_qeglobals.d_lpMruMenu = CreateMruMenuDefault();
    }

    // 1b) The main icon TOOLBAR (CMainFrame::OnCreate 0x420ac6: CreateEx + LoadToolBar(152)).
    //     Button order + icon strip are the binary's (RT_TOOLBAR/RT_BITMAP 152).  Docked at the
    //     top so the GetClientRect below returns the area beneath it.
    if ( m_wndToolBar.CreateEx( this, TBSTYLE_FLAT,
             WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC )
         && m_wndToolBar.LoadToolBar( IDR_TOOLBAR152 ) )
    {
        m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );
        EnableDocking( CBRS_ALIGN_ANY );
        DockControlBar( &m_wndToolBar );
        RecalcLayout();

        // Toolbar button states (OnCreate 0x420be2..0x42102e): the binary converts the toggle
        // buttons to TBSTYLE_CHECK (SetButtonInfo style 2) then seeds their check state
        // (GetToolBarCtrl_HideButton in the IDB is a misnamed TB_CHECKBUTTON wrapper).
        // CommandToIndex(-1) skips any id absent from this build's bar.
        static const UINT s_checkBtns[] = {
            32817, 32783, 32810, 32813, 32814, 32815, 32816,   // cubicclip, clipper, rotate, scale, lockX/Y/Z
            32852, 32857, 32858, 32865, 32872,                 // dontcurve, wireframe, weld, drilldown, redisperse
            32936, 33138,                                      // camera-move-mode, texture-alpha
        };
        for ( UINT id : s_checkBtns )
        {
            if ( m_wndToolBar.CommandToIndex( id ) < 0 )
                continue;                                       // not on this build's bar
            TBBUTTONINFO tbi;
            memset( &tbi, 0, sizeof( tbi ) );
            tbi.cbSize  = sizeof( tbi );
            tbi.dwMask  = TBIF_STYLE;
            tbi.fsStyle = TBSTYLE_CHECK;                        // = the IDB's SetButtonInfo style 2
            m_wndToolBar.SendMessage( TB_SETBUTTONINFO, id, (LPARAM)&tbi );
        }
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32814, FALSE );                       // scale-lock X off
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32815, FALSE );                       // scale-lock Y off
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32816, FALSE );                       // scale-lock Z off
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32858, TRUE );                        // patch weld on (default)
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32865, TRUE );                        // patch drill-down on (default)
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32936, g_PrefsDlg->camera_mode   != 0 ); // camera movement mode
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 33138, g_PrefsDlg->camera_masked != 0 ); // texture alpha rendering
    }

    // 0x4210bc: clearing m_bAutoMenuEnable stops MFC auto-enabling/checking toolbar buttons at
    // idle - the editor manages those states explicitly (TB_CHECKBUTTON in each handler).
    m_bAutoMenuEnable = FALSE;

    // 2) The docked texture bar across the top strip (hand-built; radiant.rc has no
    //    template).  Created before the QE4 layout so the views inset below it.
    CRect rc;
    GetClientRect( &rc );
    if ( rc.Width() < 64 || rc.Height() < 64 )
        rc.SetRect( 0, 0, 1280, 800 );
    int barH = CTextureBar::CreateBar( this, &m_wndTextureBar, rc.Width() );

    // OnCreate 0x4211b8: the binary shows the texture bar only when g_PrefsDlg->m_bTextureBar
    // is set (ShowControlBar), and it defaults OFF - the Tex/SH/SV/Scale/Rot fields are the
    // on-demand Surface Inspector, not a permanent strip.  Reclaim its height when hidden.
    if ( !g_PrefsDlg->m_bTextureBar )
    {
        if ( m_wndTextureBar.GetSafeHwnd() )
            m_wndTextureBar.ShowWindow( SW_HIDE );
        barH = 0;
    }

    // 3) The five render windows + entity placeholder, laid out QE4-style below the bar.
    EdLayout L = Radiant_ComputeLayout( rc.Width(), rc.Height(), barH );
    if ( !Radiant_CreateRenderWindows( this, L ) )
    {
        Radiant_FL_Log( "render-window creation FAILED" );
        return -1;
    }

    // 3) The real renderer bring-up: R_InitRenderCommands + hunk + R_InitEditor +
    //    R_InitRendererForWindow×5 + fonts/qerfont + utility materials. Camera is the
    //    first window → it becomes the D3D device window; the rest are additional swap
    //    chains. (Views select their target by HWND, so the device-window choice is free.)
    Radiant_FL_Log( "R_BeginRegistrationInternal..." );
    R_BeginRegistrationInternal();
    g_radiantFirstLightRendererReady = true;
    Radiant_FL_Log( "renderer up (windowCount=%d, font=%p)",
        dx.windowCount, (void *)g_qeglobals.d_font_list );

    // 4) Menu + accelerator (the real IDR_MENU_QUAKE3 / IDR_MAIN_ACCEL from the .rc).
    HMENU hMenu = LoadMenu( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDR_MENU_QUAKE3 ) );
    if ( hMenu )
        ::SetMenu( m_hWnd, hMenu );
    m_hAccel = LoadAccelerators( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDR_MAIN_ACCEL ) );
    Radiant_FL_Log( "menu=%p accel=%p", (void *)hMenu, (void *)m_hAccel );

    // 4a) User keyboard command map (binary OnCreate 0x4210bf/0x4210c9): layer the radiant.ini
    //     [Commands] overrides onto the built-in defaults, then annotate the menu items with
    //     their current key bindings.  With no radiant.ini the defaults (g_radiantCommandsDefault)
    //     are unchanged — exactly the binary's missing-file behaviour.
    LoadCommandMap();
    if ( hMenu )
        ShowMenuItemKeyBindings( hMenu );

    // 4b) Build the Textures-menu Usage / Locale / Surface-type filter submenus (the binary's
    //     FillTextureMenu, from QE_LoadProject / SetButtonMenuStates): loads
    //     ../deffiles/materials/{usage,locale}.txt (TexFilter_LoadMenuFile 0x45B010) and appends
    //     three CreateMenu() popups to the Textures menu (GetSubMenu(menu,5)).  MUST run AFTER
    //     ::SetMenu so GetMenu(d_hwndMain) is live.
    FillTextureMenu();

    // 4c) Populate the File→Recent Files (MRU) menu from the registry (binary
    //     SetButtonMenuStates 0x420039: if d_project_entity, LoadMruInReg(d_lpMruMenu) +
    //     MRU_InsertItem into GetSubMenu(GetMenu, 0) = the File popup).  The recent-files
    //     items become clickable command ids 8001..8009 → CMainFrame::OnMru → DoMru.
    if ( g_qeglobals.d_lpMruMenu && g_qeglobals.d_project_entity )
    {
        extern void LoadMruInReg( LPMRUMENU *mru );                 // qe3.cpp 0x48A870
        extern void MRU_InsertItem( LPMRUMENU *mru, HMENU hMenu );  // qe3.cpp 0x48A400
        LoadMruInReg( g_qeglobals.d_lpMruMenu );
        MRU_InsertItem( g_qeglobals.d_lpMruMenu, GetSubMenu( GetMenu()->GetSafeHmenu(), 0 ) );
        Radiant_FL_Log( "MRU loaded: %d recent files", g_qeglobals.d_lpMruMenu->wNbItemFill );
    }

    // 5) Status bar.
    Radiant_CreateStatusBar( this );

    // 6) Bootstrap an empty map (sentinel lists) so File→Open / a cmdline load have a
    //    valid map state to free+reload (the binary does Map_New inside QE_LoadProject).
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );

    // 6a-pre) The head of Load_Textures (0x45d140) that the port skips by calling
    //     Load_Materials() directly: seed g_qeglobals.random_texture_stuff[0..2] (the per-edit-
    //     layer "current texdef" a new brush's faces are stamped from) with the default
    //     materials + sample sizes.  Without it the current texdef stays ZERO (radMtl == NULL),
    //     so a brush drawn before the first texture-window click gets a degenerate MaterialDef -
    //     it renders untextured AND is unpickable.
    {
        extern void SetMaterial( const char *name, patchMesh_material *out );          // materialdef.cpp 0x4315c0
        extern int  Init_MaterialLayer( MaterialDef *channel, MaterialDef *src );      // materialdef.cpp 0x472c00
        curTexWndLayer_t *rts = g_qeglobals.random_texture_stuff;
        Get_MaterialNames();       // 0x45d143
        rts[0].sampleSize = 0.25f;   // 0x45d14e
        rts[1].sampleSize = 16.0f;   // 0x45d164
        g_qeglobals.current_edit_layer = 0;   // 0x45d16a
        rts[2].sampleSize = 0.25f;   // 0x45d174
        SetMaterial( "$default",       (patchMesh_material *)&rts[0].mtl );   // 0x45d17a
        SetMaterial( "lightmap_gray",  (patchMesh_material *)&rts[1].mtl );   // 0x45d189
        SetMaterial( "smoothing_hard", (patchMesh_material *)&rts[2].mtl );   // 0x45d198
        // Init_MaterialLayer's 2nd arg is the sample-size FLOAT BITS reinterpreted as a ptr
        // (cf. map.cpp / Face_InitMaterialChannel).
        float s0 = 0.25f, s1 = 16.0f, s2 = 0.25f;
        Init_MaterialLayer( &rts[0].mtl, *(MaterialDef **)&s0 );   // 0x45d1ac
        Init_MaterialLayer( &rts[1].mtl, *(MaterialDef **)&s1 );   // 0x45d1bf
        Init_MaterialLayer( &rts[2].mtl, *(MaterialDef **)&s2 );   // 0x45d1cf
    }

    // 6a-bis) Bulk-load every material in the searchpaths into the browser (the binary's
    //     Load_Textures->Load_Materials, run from QE_LoadProject).  Render handles bind lazily
    //     as thumbnails scroll into view.
    Load_Materials();

    // 6a-ter) Scan the AI-type + gametype def folders, then the weapon defs, into the eclass
    //     list - the binary's QE_LoadProject runs Load_Defs("aitype") /
    //     Load_Defs("maps/mp/gametypes") / ScanWeapAiFiles right after
    //     Eclass_InitForSourceDirectory, so those entities appear in the entity RMB menu.
    {
        extern eclass_t *Eclass_InitForSourceDirectory( char *path );  // eclass.cpp (0x481B50)
        extern char     *ValueForKey2( int e, const char *key );       // entity.cpp 0x4825C0
        extern void      Load_Defs( const char *folder );              // eclass.cpp (0x48B6C0)
        extern void      ScanWeapAiFiles();                            // eclass.cpp (0x48BA40)
        extern const char *Dvar_GetString( const char *dvarName );     // qcommon (fs_basepath)
        // QE_LoadProject (0x48bab0) tail: the binary reads the project entity's "entitypath"
        // epair (stock cod4.prj: ".\cod4.def") and scans it FIRST via
        // Eclass_InitForSourceDirectory - which Eclass_FreeAll()s and resets g_eclass, so it
        // MUST precede the appending Load_Defs calls.
        // KISAK: the binary _findfirst's ".\cod4.def" relative to the process CWD, relying on
        // that being the install bin\.  The port's CWD is the build-output dir, so resolve
        // entitypath against <fs_basepath>\bin\ and only run the FreeAll-ing scan when the
        // resolved file EXISTS - else a missing cod4.def would WIPE the palette.
        char *entitypath = g_qeglobals.d_project_entity
            ? ValueForKey2( (int)(intptr_t)g_qeglobals.d_project_entity, "entitypath" )
            : (char *)"";
        char resolvedDef[MAX_PATH] = "";
        if ( entitypath && *entitypath )
        {
            const char *rel = entitypath;
            if ( rel[0] == '.' && ( rel[1] == '\\' || rel[1] == '/' ) )   // strip a leading ".\"
                rel += 2;
            const bool absolute = ( rel[0] == '\\' || rel[0] == '/' || ( rel[0] && rel[1] == ':' ) );
            const char *base = Dvar_GetString( "fs_basepath" );
            if ( absolute )
                _snprintf( resolvedDef, sizeof( resolvedDef ), "%s", entitypath );
            else if ( base && *base )
                _snprintf( resolvedDef, sizeof( resolvedDef ), "%s\\bin\\%s", base, rel );
            else
                _snprintf( resolvedDef, sizeof( resolvedDef ), "%s", entitypath );
            resolvedDef[sizeof( resolvedDef ) - 1] = 0;
        }
        if ( resolvedDef[0] && GetFileAttributesA( resolvedDef ) != INVALID_FILE_ATTRIBUTES )
            Eclass_InitForSourceDirectory( resolvedDef );   // loads cod4.def (base palette)
        else
            Radiant_FL_Log( "Eclass: base entitypath '%s' -> '%s' not found; keeping existing eclasses",
                            entitypath ? entitypath : "", resolvedDef );
        Load_Defs( "aitype" );
        Load_Defs( "maps/mp/gametypes" );
        ScanWeapAiFiles();
    }

    // 6b) Load the visibility filters (RadiantFilters.txt -> the 4 category lists); the binary
    //     does this in OnCreate (0x420a5e).  CLEAN cases only (4/5/8) - face + materialdef-
    //     coupled cases (3/6/7) are dropped at load (see filters.cpp), so FilterBrush never
    //     evaluates a parked case. [STALE as of the filters unit: cases 3/6/7
    //     (FilterCond_Material / _Misc / _SurfaceFlag) are all implemented in
    //     filters.cpp and RadiantFilters parses every category — nothing is dropped.]
    Load_RadiantFilters();
    Radiant_RefreshFilterPane();    // populate the Filters inspector checklists now they're loaded

    // Optional cmdline map (File→Open replaces this for interactive use).
    const char *mapPath = ( __argc > 1 ) ? __argv[1] : nullptr;
    if ( Radiant_PathLooksLikeMap( mapPath ) )
        Radiant_OpenMap( mapPath );

    // No map loaded → the Map_NewMap() bootstrap above left world_entity NULL, and the
    // first drag-created brush would NULL-deref Entity_LinkBrush( world_entity->
    // def ) in Ed_NewBrushDrag (the "draw a brush on the empty startup
    // map" crash).  The binary ends startup with a valid worldspawn (QE_LoadProject →
    // Map_New); match that so the empty startup map is immediately editable.  (A successful
    // cmdline load already set world_entity, so this only fires for the no-map case; File→New
    // uses the same Map_New().)
    if ( !world_entity )
        Map_New();

    // Map load ends in Texture_ShowInuse (clears is_in_use for everything the map doesn't
    // reference), which would hide the bulk-loaded library behind the in-use filter.  Default
    // the texture browser to SHOW ALL on startup so the full material library is browsable
    // immediately (the user can still toggle Textures→Show In Use / Ctrl-A).
    Texture_ShowAll();

    // SetButtonMenuStates 0x420000 runs CheckTextureScale after project load.  Its
    // Texture_ResetPosition tail selects the first visible material at (9,9), making that
    // material the current brush texture before the user draws the first brush.
    switch ( g_PrefsDlg->m_nTextureWindowScale )
    {
        case 10:  CheckTextureScale( 32898 ); break;
        case 25:  CheckTextureScale( 32897 ); break;
        case 50:  CheckTextureScale( 32896 ); break;
        case 200: CheckTextureScale( 32894 ); break;
        default:  CheckTextureScale( 32895 ); break;
    }

    QE_CountBrushesAndUpdateStatusBar();
    SetGridStatus();                // initial grid status (pane 4)
    Radiant_CheckGridMenu( this );  // radio-check the default grid-size menu item
    // Seed the Light-Preview submenu check marks from the LOADED prefs (the toggle handlers
    // only set them on click, and enable_light_preview DEFAULTS TO 1).
    Radiant_CheckMenu( this, 33950, g_PrefsDlg->enable_light_preview != 0 );
    Radiant_CheckMenu( this, 36108, g_PrefsDlg->preview_sun_aswell   != 0 );

    m_bDoLoop = true;
    SetTimer( 1, 250, NULL );       // periodic status refresh (the binary's 1s loop timer)
    Radiant_FL_Log( "CMainFrame::OnCreate done" );
    return 0;
}

// Lay the QE4 panes out into the frame client area (minus the docked status bar + texture
// bar) using the current splitter fractions.  Called from OnSize and from the splitter-bar
// drag (OnMouseMove).
void CMainFrame::RelayoutPanes()
{
    CRect rc; GetClientRect( &rc );
    int cx = rc.Width(), cy = rc.Height();
    if ( cx < 64 || cy < 64 )
        return;
    // The status bar (a docked control bar) owns the bottom strip; lay the panes out in
    // the remaining client area.
    int sbH = 0;
    if ( m_wndStatusBar.GetSafeHwnd() )
    {
        CRect sbr; m_wndStatusBar.GetWindowRect( &sbr );
        sbH = sbr.Height();
    }
    int useH = cy - sbH;
    if ( useH < 64 ) useH = cy;

    // Reposition the docked texture bar across the top, then inset the QE4 layout below it.
    CTextureBar::LayoutBar( cx );

    EdLayout L = Radiant_ComputeLayout( cx, useH, g_texBarHeight );
    if ( m_pXYWnd  && m_pXYWnd->GetSafeHwnd()  ) m_pXYWnd->MoveWindow ( &L.xy  );
    if ( m_pCamWnd && m_pCamWnd->GetSafeHwnd() ) m_pCamWnd->MoveWindow( &L.cam );
    if ( m_pZWnd   && m_pZWnd->GetSafeHwnd()   ) m_pZWnd->MoveWindow  ( &L.z   );
    if ( m_pTexWnd && m_pTexWnd->GetSafeHwnd() ) m_pTexWnd->MoveWindow( &L.tex );
    // m_pEntWnd is a FLOATING window (toggled by N/O/T/F) — not laid out by the dock.
    if ( m_wndConsole.GetSafeHwnd() )            m_wndConsole.MoveWindow( &L.con );
}

void CMainFrame::OnSize( UINT nType, int cx, int cy )
{
    CFrameWnd::OnSize( nType, cx, cy );
    if ( cx < 64 || cy < 64 )
        return;   // ignore minimize — keep the swap chains at their last real size
    RelayoutPanes();
}

// Splitter-bar hit-test on the GAP gutters (client coords): 0 = none; 1/2 = vertical
// (Z|XY, XY|right); 3/4 = horizontal (cam|ent, ent|tex) inside the right column; 5 =
// horizontal (views|console).  Geometry is cached by Radiant_ComputeLayout.
int CMainFrame::HitTestBar( int x, int y )
{
    const int TOL = GAP + 3;
    if ( y >= s_barViewsTop && y <= s_barViewsBot )
    {
        if ( abs( x - s_barV1x ) <= TOL ) return 1;
        if ( abs( x - s_barV2x ) <= TOL ) return 2;
    }
    if ( x >= s_barRightL && x <= s_barRightR )
    {
        if ( abs( y - s_barH1y ) <= TOL ) return 3;
        if ( abs( y - s_barH2y ) <= TOL ) return 4;
    }
    if ( abs( y - s_barH3y ) <= TOL && x >= 0 && x <= s_barCx ) return 5;
    return 0;
}

BOOL CMainFrame::OnSetCursor( CWnd *pWnd, UINT nHitTest, UINT message )
{
    if ( pWnd == this && nHitTest == HTCLIENT )
    {
        CPoint p; GetCursorPos( &p ); ScreenToClient( &p );
        int b = HitTestBar( p.x, p.y );
        if ( b == 1 || b == 2 ) { ::SetCursor( ::LoadCursor( NULL, IDC_SIZEWE ) ); return TRUE; }
        if ( b >= 3 )           { ::SetCursor( ::LoadCursor( NULL, IDC_SIZENS ) ); return TRUE; }
    }
    return CFrameWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CMainFrame::OnLButtonDown( UINT nFlags, CPoint point )
{
    int b = HitTestBar( point.x, point.y );
    if ( b )
    {
        s_dragBar = b;
        SetCapture();
        return;
    }
    CFrameWnd::OnLButtonDown( nFlags, point );
}

void CMainFrame::OnMouseMove( UINT nFlags, CPoint point )
{
    if ( s_dragBar && ( nFlags & MK_LBUTTON ) && s_barCx > 0 && s_barTopH > 0 && s_barInnerH > 0 )
    {
        switch ( s_dragBar )
        {
            case 1: s_fracZ       = Clampf( (float)point.x / s_barCx, 0.02f, 0.40f ); break;
            case 2: s_fracRight   = Clampf( (float)( s_barCx - point.x ) / s_barCx, 0.12f, 0.85f ); break;  // max was 0.50 — that floored the XY view at ~50% width; binary's CSplitterWnd has no such cap
            case 3: s_fracCam     = Clampf( (float)( point.y - s_barTopInset ) / s_barTopH, 0.15f, 0.80f ); break;
            case 4: s_fracEnt     = Clampf( (float)( point.y - s_barH1y ) / s_barTopH, 0.08f, 0.70f ); break;
            case 5: s_fracConsole = Clampf( (float)( s_barInnerH - ( point.y - s_barTopInset ) ) / s_barInnerH, 0.05f, 0.80f ); break;  // max was 0.50 — let the XY view's height shrink past 50% too
        }
        RelayoutPanes();
        return;
    }
    CFrameWnd::OnMouseMove( nFlags, point );
}

void CMainFrame::OnLButtonUp( UINT nFlags, CPoint point )
{
    if ( s_dragBar )
    {
        s_dragBar = 0;
        ReleaseCapture();
        return;
    }
    CFrameWnd::OnLButtonUp( nFlags, point );
}

void CMainFrame::OnTimer( UINT_PTR nIDEvent )
{
    if ( nIDEvent == 1 )
        QE_CountBrushesAndUpdateStatusBar();   // refresh counts; flushing is RoutineProcessing's job
    CFrameWnd::OnTimer( nIDEvent );
}

// ── The real invalidation broadcast (CMainFrame::UpdateWindows, IDB 0x427090) ────
// RedrawWindow each view whose W_* bit is set. RDW_UPDATENOW forces a synchronous repaint
// (so drags track the cursor). m_bCamPreview is implicit-true here.
void CMainFrame::UpdateWindows( int nBits )
{
    if ( ( nBits & ( W_XY | W_XY_OVERLAY ) ) && m_pXYWnd && m_pXYWnd->GetSafeHwnd() )
        ::RedrawWindow( m_pXYWnd->m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
    if ( ( nBits & ( W_CAMERA | W_CAMERA_IFON ) ) && m_pCamWnd && m_pCamWnd->GetSafeHwnd() )
        ::RedrawWindow( m_pCamWnd->m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
    if ( ( nBits & ( W_Z | W_Z_OVERLAY ) ) && m_pZWnd && m_pZWnd->GetSafeHwnd() )
        ::RedrawWindow( m_pZWnd->m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
    if ( ( nBits & W_TEXTURE ) && m_pTexWnd && m_pTexWnd->GetSafeHwnd() )
        ::RedrawWindow( m_pTexWnd->m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );

    // surfinsp: refresh the Surface Inspector edit fields whenever a view is invalidated
    // (selection / texdef edit / map load all set g_nUpdateBits).  No-op when it's closed.
    Surf_UpdateInspector();

    // texture bar: refresh its texdef readout + material name on the same invalidation
    // broadcast (the binary refreshes it from UpdateTextureBar on every current-texture
    // change).  No-op when the bar isn't up.
    if ( m_wndTextureBar.GetSafeHwnd() )
        CTextureBar::GetSurfaceAttributes( &m_wndTextureBar );
}

// Idle pump (CMainFrame::RoutineProcessing 0x421a90), driven each idle from
// CRadiantApp::OnIdle: compute dtime from clock() into g_qeglobals.g_time/g_oldtime (clamp
// dt>2 -> 0.1, then dtime>0.2 -> 0.2), pump Cam_MouseControl for the RMB cursor-joystick fly,
// then flush g_nUpdateBits via UpdateWindows.
extern void Cam_MouseControl( CCamWnd *cam, float dtime );   // camwnd.cpp (0x403950)
void CMainFrame::RoutineProcessing()
{
    if ( !m_bDoLoop )
        return;

    // dtime = wall-clock seconds since last pump, clamped (binary 0x421ada..0x421b24).
    clock_t now = clock();
    double dtime = (double)now / 1000.0 - g_qeglobals.g_time;
    g_qeglobals.g_time = (double)now / 1000.0;
    if ( dtime > 2.0 )
        dtime = 0.1;
    g_qeglobals.g_oldtime = dtime;
    if ( dtime > 0.2 )
        dtime = 0.2;
    if ( m_pCamWnd )
        Cam_MouseControl( m_pCamWnd, (float)dtime );

    if ( g_nUpdateBits )
    {
        int bits = g_nUpdateBits;
        g_nUpdateBits = 0;
        UpdateWindows( bits );
    }
}

// Editor hotkey command map (CMainFrame::OnKeyDown 0x422370 + LoadCommands_stdmap 0x420140
// over g_Commands @0x73B240, 187 entries).  Radiant binds single-key + modified editor
// shortcuts through this map, NOT the MFC accelerator table: OnKeyDown builds a modifier mask
// (Shift=1, Alt=2, Ctrl=4, Win=8), looks the key up, and SendMessage(WM_COMMAND, commandId).
// std::map keeps the FIRST binding for a duplicate key, so a forward first-match search
// matches the binary.  g_radiantCommands is a MUTABLE copy of g_radiantCommandsDefault (the
// binary's table, verbatim): LoadCommandMap (0x421230) overrides individual entries BY NAME
// from radiant.ini [Commands]; ShowMenuItemKeyBindings (0x420460) annotates the menu items.
struct RadiantCommand { const char *name; unsigned char vk; unsigned char mods; int commandId; };
static const RadiantCommand g_radiantCommandsDefault[] = {
    { "ToggleOutlineDraw", 0x4A, 0, 33103 },
    { "ToggleTintDraw", 0x4A, 1, 33172 },
    { "CSGMerge", 0x55, 4, 32927 },
    { "AutoCaulk", 0x43, 2, 33220 },
    { "ViewFilters", 0x46, 0, 33104 },
    { "HideSelected", 0x48, 0, 32923 },
    { "HideUnSelected", 0x48, 2, 32934 },
    { "ShowHidden", 0x48, 1, 32924 },
    { "ShowLastHidden", 0x48, 5, 33246 },
    { "FitBrush", 0x42, 1, 33098 },
    { "AdvancedCurveEdit", 0x59, 0, 33130 },
    { "SelectionKeyValue", 0x46, 5, 33133 },
    { "ToggleLockPatchVertices", 0xBE, 4, 33140 },
    { "ToggleUnlockPatchVertices", 0xBE, 5, 33139 },
    { "ToggleTurnTerrainEdges", 0xBF, 4, 33141 },
    { "ShowTexturesInUse", 0x55, 0, 32974 },
    { "ViewTextures", 0x54, 0, 33018 },
    { "ThickenPatch", 0x54, 5, 32904 },
    { "AddTerrainRow", 0x41, 5, 33153 },
    { "ExtrudeTerrainRow", 0x4F, 2, 33192 },
    { "RemoveTerrainRow", 0x51, 5, 33154 },
    { "SplitPatch", 0x58, 5, 33158 },
    { "SurfaceInspector", 0x53, 0, 33041 },
    { "PatchInspector", 0x53, 1, 33092 },
    { "ApplyPatchCap", 0x50, 5, 35040 },
    { "TolerantWeld", 0x4A, 5, 33155 },
    { "RedisperseVertices", 0x46, 1, 33170 },
    { "RedisperseRows", 0x45, 1, 32888 },
    { "RedisperseCols", 0x45, 5, 32889 },
    { "InvertCurveTextureX", 0x49, 5, 32903 },
    { "InvertCurveTextureY", 0x49, 1, 32899 },
    { "InvertCurve", 0x49, 4, 32881 },
    { "IncPatchColumn", 0x6B, 5, 32868 },
    { "IncPatchRow", 0x6B, 4, 32867 },
    { "DecPatchColumn", 0x6D, 5, 32870 },
    { "DecPatchRow", 0x6D, 4, 32869 },
    { "Patch TAB", 0x09, 0, 33089 },
    { "Patch TAB", 0x09, 1, 33089 },
    { "TogglePatchWireframes", 0x57, 1, 32857 },
    { "SelectNudgeDown", 0x28, 2, 32850 },
    { "EntityColor", 0x4B, 0, 33036 },
    { "CameraForward", 0x26, 0, 33059 },
    { "CameraBack", 0x28, 0, 33060 },
    { "CameraLeft", 0x25, 0, 33057 },
    { "CameraRight", 0x27, 0, 33058 },
    { "Vertex Select Up", 0x26, 4, 33165 },
    { "Vertex Select Down", 0x28, 4, 33166 },
    { "CameraUp", 0x44, 0, 33055 },
    { "CameraDown", 0x43, 0, 33056 },
    { "CameraAngleUp", 0x41, 0, 33061 },
    { "CameraAngleDown", 0x5A, 0, 33062 },
    { "CameraStrafeRight", 0xBE, 0, 33064 },
    { "CameraStrafeLeft", 0xBC, 0, 33063 },
    { "ToggleGrid", 0x30, 0, 33065 },
    { "SetGridPointFive", 0xC0, 0, 35021 },
    { "SetGrid1", 0x31, 0, 35022 },
    { "SetGrid2", 0x32, 0, 35023 },
    { "SetGrid4", 0x33, 0, 35024 },
    { "SetGrid8", 0x34, 0, 35025 },
    { "SetGrid16", 0x35, 0, 35026 },
    { "SetGrid32", 0x36, 0, 35027 },
    { "SetGrid64", 0x37, 0, 35029 },
    { "SetGrid256", 0x38, 0, 35032 },
    { "SetGrid512", 0x39, 0, 35033 },
    { "DragEdges", 0x45, 0, 33006 },
    { "DragVertices", 0x56, 0, 33005 },
    { "ViewEntityInfo", 0x4E, 0, 33017 },
    { "ViewConsole", 0x4F, 0, 33016 },
    { "CloneSelection", 0x20, 0, 33001 },
    { "DeleteSelection", 0x08, 0, 33003 },
    { "UnSelectSelection", 0x1B, 0, 33002 },
    { "InvertSelection", 0x49, 0, 33101 },
    { "CenterView", 0x23, 0, 32953 },
    { "ZoomOut", 0x2D, 0, 32996 },
    { "ZoomIn", 0x2E, 0, 32995 },
    { "SelectPrev", 0xBC, 1, 33161 },
    { "SelectNext", 0xBE, 1, 33160 },
    { "UpFloor", 0x21, 0, 32954 },
    { "DownFloor", 0x22, 0, 32955 },
    { "LinkSelectionToggle", 0x4F, 1, 1085 },
    { "ToggleClipper", 0x58, 0, 32783 },
    { "ToggleCrosshairs", 0x58, 1, 33100 },
    { "ToggleTexMoveLock", 0x54, 1, 32785 },
    { "ToggleTexRotateLock", 0x52, 1, 32835 },
    { "ToggleLightmapLock", 0x00, 0, 33237 },
    { "RemoveColorNode", 0x52, 4, 10 },
    { "ToggleLayers", 0x4C, 0, 33954 },
    { "Preferences", 0x50, 0, 32784 },
    { "ToggleCamera", 0x43, 5, 33069 },
    { "ToggleView", 0x56, 5, 33071 },
    { "DropVertices", 0x44, 7, 33213 },
    { "RotateZ", 0x44, 1, 32961 },
    { "ToggleZ", 0x5A, 5, 33070 },
    { "SameTargetname", 0x42, 0, 36121 },
    { "SameTarget", 0x42, 4, 36123 },
    { "ConnectSelection", 0x57, 0, 33021 },
    { "SetViewToEntity", 0x75, 0, 33210 },
    { "SplaySelection", 0x57, 2, 33157 },
    { "SelectConnectedEntities", 0x45, 6, 33134 },
    { "SelectTargettedEntities", 0x45, 4, 36110 },
    { "PatchMatrixTranspose", 0x4D, 5, 32906 },
    { "MakeDetail", 0x44, 5, 33042 },
    { "MakeWeaponClip", 0x57, 5, 196 },
    { "MakeNonColliding", 0xBD, 5, 197 },
    { "MakeSplitCoplanarGeo", 0x00, 0, 33223 },
    { "MakeDontSplitCoplanarGeo", 0x00, 0, 33224 },
    { "MapInfo", 0x4D, 0, 32786 },
    { "NextLeakSpot", 0x4B, 5, 33024 },
    { "PrevLeakSpot", 0x4C, 5, 33025 },
    { "FileOpen", 0x4F, 4, 57601 },
    { "FileSave", 0x53, 4, 57603 },
    { "Quit", 0x00, 0, 32951 },
    { "NextView", 0x09, 4, 32789 },
    { "ClipSelected", 0x0D, 0, 32795 },
    { "SplitSelected", 0x0D, 1, 32794 },
    { "FlipClip", 0x0D, 4, 32796 },
    { "MouseRotate", 0x52, 0, 32810 },
    { "Copy", 0x43, 4, 33039 },
    { "Paste", 0x56, 4, 33040 },
    { "Undo", 0x5A, 4, 57643 },
    { "Redo", 0x59, 4, 57644 },
    { "ZZoomOut", 0x2D, 4, 33000 },
    { "ZZoomIn", 0x2E, 4, 32999 },
    { "TexDecrement", 0x6D, 1, 33072 },
    { "TexIncrement", 0x6B, 1, 33073 },
    { "TextureFit", 0x46, 4, 33074 },
    { "TextureFitAll", 0x46, 6, 33234 },
    { "TexRotateClock", 0x25, 4, 33075 },
    { "TexRotateCounter", 0x27, 4, 33076 },
    { "TexShiftLeft", 0x25, 1, 33079 },
    { "TexShiftRight", 0x27, 1, 33080 },
    { "TexShiftUp", 0x26, 1, 33081 },
    { "TexShiftDown", 0x28, 1, 33082 },
    { "TexLayerCycle", 0x4C, 1, 33238 },
    { "TexLayerMaterial", 0x00, 0, 33232 },
    { "TexLayerLightmap", 0x00, 0, 33233 },
    { "GridDown", 0xDB, 0, 33084 },
    { "GridUp", 0xDD, 0, 33083 },
    { "TexScaleLeft", 0x25, 4, 33085 },
    { "TexScaleRight", 0x27, 4, 33086 },
    { "LightShiftUp", 0xDD, 2, 33145 },
    { "LightShiftDown", 0xDB, 2, 33146 },
    { "CyclinderHeightUp", 0xBE, 2, 33176 },
    { "CyclinderHeightDown", 0xBC, 2, 33177 },
    { "AssociateEntities", 0x47, 1, 33150 },
    { "VehicleGroup", 0x56, 1, 33221 },
    { "DynEntities", 0x59, 1, 36106 },
    { "DisassociateEntities", 0x47, 5, 33151 },
    { "SelectedAssociated", 0x58, 4, 33152 },
    { "OverBrightShiftUp", 0xDD, 1, 33147 },
    { "OverBrightShiftDown", 0xDB, 1, 33148 },
    { "CubicClipZoomOut", 0xDD, 4, 32819 },
    { "CubicClipZoomIn", 0xDB, 4, 32820 },
    { "ToggleCubicClip", 0xDC, 4, 32817 },
    { "MoveSelectionDOWN", 0x6D, 0, 32829 },
    { "MoveSelectionUP", 0x6B, 0, 32831 },
    { "LinkSelected", 0x51, 2, 33211 },
    { "SelectNudgeLeft", 0x25, 2, 32847 },
    { "GetDistance", 0x70, 2, 33178 },
    { "AutoEdgeTurn", 0x71, 2, 33179 },
    { "SelectNudgeRight", 0x27, 2, 32848 },
    { "SelectNudgeUp", 0x26, 2, 32849 },
    { "CycleCapTexturePatch", 0x4E, 5, 32905 },
    { "NaturalizePatch", 0x4E, 4, 32890 },
    { "ToggleSnapToGrid", 0x47, 6, 32793 },
    { "SelectSnapPointsToGrid", 0x47, 4, 33091 },
    { "ShowAllTextures", 0x41, 4, 32973 },
    { "SelectAllOfType", 0x41, 1, 33093 },
    { "SelectAllOfTypeRecurse", 0x41, 2, 33212 },
    { "CapCurrentCurve", 0x43, 1, 32885 },
    { "MakeStructural", 0x53, 5, 33043 },
    { "Center2DOnCamera", 0x58, 2, 33108 },
    { "EnterPrefab", 0x22, 2, 33173 },
    { "LeavePrefab", 0x21, 2, 33174 },
    { "LightPreviewToggle", 0x77, 0, 33950 },
    { "LightPreviewStart", 0x77, 1, 33951 },
    { "LightPreviewStop", 0x77, 3, 33952 },
    { "LightPreviewClear", 0x77, 2, 33953 },
    { "LightPreviewSun", 0x77, 4, 36108 },
    { "LightPreviewRegions", 0x77, 5, 36125 },
    { "MaxLightIntensity", 0x77, 6, 36122 },
    { "VertEdit", 0x47, 0, 33199 },
    { "RefreshTextures", 0x74, 0, 33204 },
    { "TogglePreviewModels", 0x00, 0, 35005 },
    { "HideByClassname", 0x48, 7, 32925 },
    { "ToggleLayeredMaterialWnd", 0x73, 0, 35008 },
    { "SaveLayeredMaterials", 0x00, 0, 35009 },
};

// Mutable runtime binding table (the port's analog of the LoadCommands_stdmap std::map).
// Seeded from the defaults; LoadCommandMap patches vk/mods in place from radiant.ini.
static RadiantCommand g_radiantCommands[ ARRAYSIZE( g_radiantCommandsDefault ) ];
static bool           g_radiantCommandsInit = false;
static void Radiant_SeedCommandTable()
{
    if ( g_radiantCommandsInit )
        return;
    memcpy( g_radiantCommands, g_radiantCommandsDefault, sizeof( g_radiantCommands ) );
    g_radiantCommandsInit = true;
}

// The named-key table (binary g_Keys @0x73BDF8, 47 entries, ends at g_KeysExceeded 0x73BF74).
// Maps a key NAME to its VK code — used by LoadCommandMap (parse an INI binding string) and by
// ShowMenuItemKeyBindings (VK → name for the menu annotation).  Extracted verbatim from the exe.
struct RadiantKeyName { const char *name; unsigned int vk; };
static const RadiantKeyName g_radiantKeys[] = {
    { "Space", 0x20 }, { "Backspace", 0x08 }, { "Escape", 0x1B }, { "End", 0x23 },
    { "Insert", 0x2D }, { "Delete", 0x2E }, { "PageUp", 0x21 }, { "PageDown", 0x22 },
    { "Up", 0x26 }, { "Down", 0x28 }, { "Left", 0x25 }, { "Right", 0x27 },
    { "F1", 0x70 }, { "F2", 0x71 }, { "F3", 0x72 }, { "F4", 0x73 }, { "F5", 0x74 },
    { "F6", 0x75 }, { "F7", 0x76 }, { "F8", 0x77 }, { "F9", 0x78 }, { "F10", 0x79 },
    { "F11", 0x7A }, { "F12", 0x7B }, { "Tab", 0x09 }, { "Return", 0x0D },
    { "Comma", 0xBC }, { "Period", 0xBE }, { "Plus", 0x6B }, { "Multiply", 0x6A },
    { "Subtract", 0x6D }, { "NumPad0", 0x60 }, { "NumPad1", 0x61 }, { "NumPad2", 0x62 },
    { "NumPad3", 0x63 }, { "NumPad4", 0x64 }, { "NumPad5", 0x65 }, { "NumPad6", 0x66 },
    { "NumPad7", 0x67 }, { "NumPad8", 0x68 }, { "NumPad9", 0x69 }, { "Minus", 0xBD },
    { "[", 0xDB }, { "]", 0xDD }, { "\\", 0xDC }, { "~", 0xC0 }, { "LWin", 0x5B },
};

// ── LoadCommandMap (0x421230) — override the default key bindings from radiant.ini ─────
//   For each command, read radiant.ini [Commands] <name>=<binding>; if present, parse the
//   modifier flags (+alt/+ctrl/+shift/+lwin) and the key (single alnum char or a g_Keys name)
//   and overwrite that command's binding.  A missing file / missing key leaves the default.
//   INI path: g_PrefsDlg->m_strUserIniPath if set, else <exe folder>\radiant.ini (g_strAppPath).
void CMainFrame::LoadCommandMap()
{
    Radiant_SeedCommandTable();

    char iniPath[MAX_PATH];
    if ( g_PrefsDlg && !g_PrefsDlg->m_strUserIniPath.IsEmpty() )
    {
        _snprintf( iniPath, sizeof( iniPath ), "%s", (LPCSTR)g_PrefsDlg->m_strUserIniPath );
    }
    else
    {
        char exeDir[MAX_PATH];
        GetModuleFileNameA( nullptr, exeDir, sizeof( exeDir ) );
        char *slash = strrchr( exeDir, '\\' );
        if ( slash ) *slash = 0;
        _snprintf( iniPath, sizeof( iniPath ), "%s\\radiant.ini", exeDir );
    }

    for ( RadiantCommand &c : g_radiantCommands )
    {
        char buf[1024];
        if ( !GetPrivateProfileStringA( "Commands", c.name, "", buf, sizeof( buf ), iniPath ) )
            continue;   // no override for this command → keep the default binding

        // Parse + strip the modifier flags (each is optional, any order).  The binary uses
        // CString::Find + Replace; a plain strstr/erase is faithful.
        unsigned char mods = 0;
        struct { const char *tok; unsigned char bit; } flags[] =
            { { "+alt", 2 }, { "+ctrl", 4 }, { "+shift", 1 }, { "+lwin", 8 } };
        for ( auto &fl : flags )
        {
            char *at = strstr( buf, fl.tok );
            if ( at )
            {
                mods |= fl.bit;
                size_t n = strlen( fl.tok );
                memmove( at, at + n, strlen( at + n ) + 1 );   // remove the token in place
            }
        }
        // trim whitespace
        char *s = buf;
        while ( *s == ' ' || *s == '\t' ) ++s;
        char *e = s + strlen( s );
        while ( e > s && ( e[-1] == ' ' || e[-1] == '\t' ) ) --e;
        *e = 0;
        _strupr( s );

        unsigned char vk = 0;
        bool have = false;
        if ( strlen( s ) == 1 && isalnum( (unsigned char)*s ) )
        {
            vk   = (unsigned char)( *s & 0x7F );   // __toascii
            have = true;
        }
        else
        {
            for ( const RadiantKeyName &k : g_radiantKeys )
            {
                // g_Keys names compared case-insensitively (binary uses _mbsicmp)
                if ( !_stricmp( s, k.name ) )
                {
                    vk   = (unsigned char)k.vk;
                    have = true;
                    break;
                }
            }
        }
        if ( have )
        {
            c.vk   = vk;
            c.mods = mods;
        }
    }
}

// ── ShowMenuItemKeyBindings (0x420460) — annotate menu items with their key binding ────
//   For each command, if a menu item with that command ID exists (and is a text item), append
//   "\t[Shift-][Alt-][Ctrl-][LWin-]KeyName" to its caption.  KeyName comes from g_Keys, or the
//   raw char (%c) for a plain-key binding.  Faithful to the binary (walks g_Commands/g_Keys).
BOOL CMainFrame::ShowMenuItemKeyBindings( HMENU hMenu )
{
    Radiant_SeedCommandTable();
    BOOL result = FALSE;
    for ( const RadiantCommand &c : g_radiantCommands )
    {
        MENUITEMINFOA mii;
        char caption[1028];
        memset( &mii, 0, sizeof( mii ) );
        mii.cbSize     = sizeof( mii );
        mii.fMask      = MIIM_TYPE;
        mii.dwTypeData = caption;
        mii.cch        = 1024;
        result = GetMenuItemInfoA( hMenu, c.commandId, FALSE, &mii );
        if ( !result || mii.fType )        // no such item / not a string item (separator etc.)
            continue;

        // drop any existing "\t..." accelerator text, then rebuild it.
        char *tab = strchr( caption, '\t' );
        if ( tab ) *tab = 0;
        char *w = caption + strlen( caption );
        *w++ = '\t';
        *w   = 0;

        if ( c.mods )
        {
            if ( c.mods & 1 ) { strcpy( w, "Shift-" ); w += strlen( w ); }
            if ( c.mods & 2 ) { strcpy( w, "Alt-" );   w += strlen( w ); }
            if ( c.mods & 4 ) { strcpy( w, "Ctrl-" );  w += strlen( w ); }
            if ( c.mods & 8 ) { strcpy( w, "LWin-" );  w += strlen( w ); }
        }

        // VK → key name (g_Keys); if not a named key, emit the raw character.
        const char *keyName = nullptr;
        for ( const RadiantKeyName &k : g_radiantKeys )
            if ( k.vk == c.vk ) { keyName = k.name; break; }
        if ( keyName )
            strcpy( w, keyName );
        else
            sprintf( caption + strlen( caption ), "%c", c.vk );

        memset( &mii, 0, sizeof( mii ) );
        mii.cbSize     = sizeof( mii );
        mii.fMask      = MIIM_TYPE;
        mii.fType      = 0;
        mii.dwTypeData = caption;
        mii.cch        = (UINT)strlen( caption );
        result = SetMenuItemInfoA( hMenu, c.commandId, FALSE, &mii );
    }
    return result;
}

// CMainFrame::OnKeyDown (0x422370) — look the pressed key+modifiers up in the command map
// and dispatch its WM_COMMAND.  Called directly from each view's OnKeyDown (the binary's
// CXYWnd/CCamWnd/CZWnd::OnKeyDown all tail-call CMainFrame::OnKeyDown(g_pParentWnd,...)).
void CMainFrame::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    (void)nRepCnt; (void)nFlags;
    // DIVERGENCE vs IDA (documented, near-unreachable): the binary's OnKeyDown 0x422370 ALSO
    // dispatches a SECOND WM_COMMAND (sub_42CD30(node)->[+32]) inside `if ( !nRepCnt && !nFlags )`.
    // Real WM_KEYDOWN always carries nRepCnt>=1, so that block never fires for genuine keypresses;
    // this port ignores nRepCnt/nFlags and omits it. Restore + characterize sub_42CD30's second
    // command only if a synthetic-key (nRepCnt==0) path is ever wired.
    TryHotkey( nChar );
}

// Command-map lookup + dispatch (factored out of OnKeyDown so the floating inspector popup can
// reach it too).  Returns true when a hotkey fired its WM_COMMAND.
bool CMainFrame::TryHotkey( UINT nChar )
{
    if ( !nChar )
        return false;
    unsigned char mods = 0;
    if ( GetKeyState( VK_MENU )    < 0 ) mods |= 2;   // Alt
    if ( GetKeyState( VK_CONTROL ) < 0 ) mods |= 4;   // Ctrl
    if ( GetKeyState( VK_SHIFT )   < 0 ) mods |= 1;   // Shift
    if ( GetKeyState( VK_LWIN )    < 0 ) mods |= 8;   // Win
    for ( const RadiantCommand &c : g_radiantCommands )
    {
        if ( c.vk == nChar && c.mods == mods )
        {
            ::SendMessageA( m_hWnd, WM_COMMAND, c.commandId, 0 );
            return true;
        }
    }
    return false;
}

// TranslateAccelerator (IDR_MAIN_ACCEL → ON_COMMAND) + the command-map Ctrl+Z/Y
// undo/redo (Radiant binds those through LoadCommandMap, not the accel table). Frame-wide
// so they work regardless of which child view has focus.
BOOL CMainFrame::PreTranslateMessage( MSG *pMsg )
{
    if ( m_hAccel && ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) )
    {
        return TRUE;
    }

    if ( pMsg->message == WM_KEYDOWN && ( GetAsyncKeyState( VK_CONTROL ) < 0 ) )
    {
        if ( pMsg->wParam == 'Z' ) { OnEditUndo(); return TRUE; }
        if ( pMsg->wParam == 'Y' ) { OnEditRedo(); return TRUE; }
    }
    return CFrameWnd::PreTranslateMessage( pMsg );
}

// ── File / Edit command wrappers ─────────────────────────────────────────────────
void CMainFrame::OnFileNew()
{
    // Faithful OnFileNew (0x423AA0): guard on OkToDiscard() (the unsaved-changes prompt,
    // now shipped — was a parked stub), pop any prefab nesting, then run the real
    // File→New = Map_New().  Map_New builds a valid worldspawn so a drag-created brush
    // can attach; the old Map_NewMap()-only path left world_entity NULL and the first
    // NewBrushDrag NULL-deref'd.
    if ( !OkToDiscard() )
        return;                            // user chose Cancel → keep the current map
    Prefab_LevelBack();
    Map_New();
    s_currentMapPath[0] = 0;
    SetWindowText( "CoD4Radiant - untitled" );
    QE_CountBrushesAndUpdateStatusBar();
    g_nUpdateBits = W_ALL;
}

void CMainFrame::OnFileOpen()
{
    // Faithful OnFileOpen (0x423ae0): guard on OkToDiscard() (then Prefab_LevelBack)
    // BEFORE popping the open dialog — so a dirty map prompts to save first.
    if ( !OkToDiscard() )
        return;
    Prefab_LevelBack();
    char file[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    memset( &ofn, 0, sizeof( ofn ) );
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner   = m_hWnd;
    ofn.lpstrFilter = "Map files (*.map)\0*.map\0All files (*.*)\0*.*\0";
    ofn.lpstrFile   = file;
    ofn.nMaxFile    = sizeof( file );
    ofn.lpstrTitle  = "Open Map";
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if ( GetOpenFileNameA( &ofn ) )
        Radiant_OpenMap( file );
}

void CMainFrame::OnFileSave()
{
    if ( !s_currentMapPath[0] )
    {
        OnFileSaveAs();
        return;
    }
    Map_SaveFile( s_currentMapPath, 0, 0 );
    Radiant_FL_Log( "saved: %s", s_currentMapPath );
}

void CMainFrame::OnFileSaveAs()
{
    OnFileSaveAs_Confirmed();              // shared body (returns save-vs-cancel)
}

void CMainFrame::OnFileExit()
{
    // File→Exit posts WM_CLOSE → OnClose runs the unsaved-changes guard (faithful: the
    // binary's exit guard lives in OnClose 0x422220, not in the Exit command).
    PostMessage( WM_CLOSE );
}

void CMainFrame::OnClose()
{
    // Faithful OnClose (0x422220): guard on OkToDiscard() (then Prefab_LevelBack) before
    // letting the frame close — so quitting with unsaved edits prompts to save.  If the
    // user cancels, swallow the close (do NOT chain to the default handler).
    if ( !OkToDiscard() )
        return;
    Prefab_LevelBack();
    CFrameWnd::OnClose();
}

// ═════════════════════════════════════════════════════════════════════════════
//  PROJECT (.prj) menu handlers + the MRU (recent-files) menu.
//  Cores (Project_Write / QE_LoadProject_ParseFile / the MRU_* family) live in qe3.cpp.
// ═════════════════════════════════════════════════════════════════════════════
extern signed int Project_Write( const char *path );                        // qe3.cpp 0x48BD90
extern const char *Project_GetCurrentPath();                                 // qe3.cpp (== dword_25D65AC)
extern LPMRUMENU  *CreateMruMenuDefault();                                    // qe3.cpp 0x48A150
extern void        MRU_NewItem( LPMRUMENU *mru, const char *lpString1 );      // qe3.cpp 0x48A2C0
extern void        MRU_InsertItem( LPMRUMENU *mru, HMENU hMenu );             // qe3.cpp 0x48A400
extern void        SaveMruInReg( LPMRUMENU *mru );                            // qe3.cpp 0x48A750
extern void        LoadMruInReg( LPMRUMENU *mru );                            // qe3.cpp 0x48A870
extern BOOL        DoMru( short nID, HWND hWnd );                             // qe3.cpp 0x4994B0
extern char       *ValueForKey2( int e, const char *key );                   // entity.cpp 0x4825C0
extern void        SetKeyValue( entity_s_def *e, const char *key, const char *value ); // entity.cpp

// ── 0x495330  ProjectDlgProc — the IDD_PROJECT_SETTINGS dialog proc ───────────
// Verbatim from the binary: WM_INITDIALOG (272) fills the 5 edit controls from the
// project entity; WM_COMMAND (273) OK reads them back (SetKeyValue) + Project_Write,
// Cancel just closes.  Control ids: 1265 basepath / 1274 mapspath / 1253 entitypath /
// 1260 game / 1273 basegame.
static INT_PTR CALLBACK ProjectDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    entity_s_def *proj = (entity_s_def *)g_qeglobals.d_project_entity;
    if ( msg == WM_INITDIALOG )
    {
        if ( !proj )
            return TRUE;
        SetDlgItemTextA( hDlg, 1265, ValueForKey2( (int)(intptr_t)proj, "basepath" ) );
        SetDlgItemTextA( hDlg, 1274, ValueForKey2( (int)(intptr_t)proj, "mapspath" ) );
        SetDlgItemTextA( hDlg, 1253, ValueForKey2( (int)(intptr_t)proj, "entitypath" ) );
        SetDlgItemTextA( hDlg, 1260, ValueForKey2( (int)(intptr_t)proj, "game" ) );
        SetDlgItemTextA( hDlg, 1273, ValueForKey2( (int)(intptr_t)proj, "basegame" ) );
        return TRUE;
    }
    if ( msg != WM_COMMAND )
        return FALSE;
    if ( LOWORD( wParam ) == IDOK )
    {
        char String[1024];
        GetDlgItemTextA( hDlg, 1265, String, 1024 ); SetKeyValue( proj, "basepath",   String );
        GetDlgItemTextA( hDlg, 1274, String, 1024 ); SetKeyValue( proj, "mapspath",   String );
        GetDlgItemTextA( hDlg, 1253, String, 1024 ); SetKeyValue( proj, "entitypath", String );
        GetDlgItemTextA( hDlg, 1260, String, 1024 ); SetKeyValue( proj, "game",       String );
        GetDlgItemTextA( hDlg, 1273, String, 1024 ); SetKeyValue( proj, "basegame",   String );
        EndDialog( hDlg, 1 );
        Project_Write( Project_GetCurrentPath() );
        return TRUE;
    }
    if ( LOWORD( wParam ) == IDCANCEL )
    {
        EndDialog( hDlg, 0 );
        return TRUE;
    }
    return FALSE;
}

// ── 0x428DE0  CMainFrame::OnFileProjectsettings ──────────────────────────────
// Pops the Project Settings dialog (edit the loaded project entity's epairs).  Faithful
// (binary DialogBoxParamA(IDD_DLG_PROJECT, ProjectDlgProc)).  No-op if no project loaded
// (the dialog would show blank fields and Project_Write would fault on a NULL entity).
void CMainFrame::OnFileProjectsettings()
{
    if ( !g_qeglobals.d_project_entity )
    {
        Radiant_FL_Log( "OnFileProjectsettings: no project loaded" );
        return;
    }
    DialogBoxParamA( g_qeglobals.d_hInstance, MAKEINTRESOURCE( IDD_PROJECT_SETTINGS ),
                     g_qeglobals.d_hwndMain, ProjectDlgProc, 0 );
}

// ── 0x426E80  CMainFrame::OnFileNewproject ───────────────────────────────────
// New Project: pick a target .prj filename (browse), strip its extension, append ".prj",
// set it as the current project path, then open Project Settings to fill in the epairs.
// The binary uses NewProjectDlg (CNewProjDlg, a filename picker) then ProjectDlgProc; here
// the filename picker is a CFileDialog save-as (behaviour-identical: obtain a target path).
void CMainFrame::OnFileNewproject()
{
    if ( !g_qeglobals.d_project_entity )
    {
        Radiant_FL_Log( "OnFileNewproject: no project loaded" );
        return;
    }
    CFileDialog dlg( FALSE, "prj", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                     "CoD4Radiant Project files (*.prj)|*.prj||", this );
    if ( dlg.DoModal() != IDOK )
        return;
    // Strip the picked path's extension and force ".prj" (binary: Com_StripExtension + ".prj").
    char Src[1028];
    _snprintf( Src, sizeof( Src ), "%s", (const char *)dlg.GetPathName() );
    Src[sizeof( Src ) - 1] = 0;
    char *dot = strrchr( Src, '.' );
    char *sl  = strrchr( Src, '\\' );
    if ( dot && ( !sl || dot > sl ) ) *dot = 0;             // Com_StripExtension
    strcat( Src, ".prj" );
    // The current-project path (binary get_m_strStatus(&dword_25D65AC, Src)) is what
    // Project_Write writes to; we set it via the project-path store in qe3.cpp by writing
    // the epairs then Project_Write inside ProjectDlgProc's OK.  Seed the path first.
    extern void Project_SetCurrentPathPublic( const char *path );
    Project_SetCurrentPathPublic( Src );
    DialogBoxParamA( g_qeglobals.d_hInstance, MAKEINTRESOURCE( IDD_PROJECT_SETTINGS ),
                     g_qeglobals.d_hwndMain, ProjectDlgProc, 0 );
}

// ── 0x49A450  DoStartupProject / 0x427010  CMainFrame::OnSetStartupProject ─────
// Change which .prj loads at startup: read the current "Prefs"/"LastProject" registry
// value, pop a "Select Project File" open dialog, and if the user picks a different file
// write it back to the registry + tell them to restart.  Verbatim from the binary
// (registry via CWinApp profile string; common dialog via GetOpenFileNameA).
void CMainFrame::OnSetStartupProject()
{
    CWinApp *app = AfxGetApp();
    CString last = app->GetProfileString( "Prefs", "LastProject", "" );

    char szFile[MAX_PATH];
    _snprintf( szFile, sizeof( szFile ), "%s", (const char *)last );
    szFile[sizeof( szFile ) - 1] = 0;

    OPENFILENAMEA ofn;
    memset( &ofn, 0, sizeof( ofn ) );
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner   = g_qeglobals.d_hwndCamera;
    ofn.lpstrFilter = "CoD4Radiant project (*.prj)\0*.prj\0";   // binary szProjectFilter
    ofn.lpstrFile   = szFile;
    ofn.nMaxFile    = MAX_PATH;
    ofn.lpstrTitle  = "Select Project File";
    ofn.Flags       = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;   // 6148=0x1804
    if ( !GetOpenFileNameA( &ofn ) )
        return;
    if ( last.Compare( szFile ) != 0 )
    {
        app->WriteProfileString( "Prefs", "LastProject", szFile );
        ::MessageBoxA( ::GetActiveWindow(),
                       "You will need to restart Radiant for this change to take affect.",
                       "Changed Startup Project", 0 );
    }
}

// ── 0x423FE0  CMainFrame::OnMru — File→Recent Files (8000-8009) ───────────────
void CMainFrame::OnMru( UINT nID )
{
    DoMru( (short)nID, m_hWnd );
}

// ── 0x421C60  CMainFrame::OnDestroy — persist the MRU list to the registry ─────
void CMainFrame::OnDestroy()
{
    if ( g_qeglobals.d_lpMruMenu )
        SaveMruInReg( g_qeglobals.d_lpMruMenu );
    CFrameWnd::OnDestroy();
}

void CMainFrame::OnEditUndo()
{
    Undo_Undo();
    QE_CountBrushesAndUpdateStatusBar();
    g_nUpdateBits = W_ALL;
}

void CMainFrame::OnEditRedo()
{
    Undo_Redo();
    QE_CountBrushesAndUpdateStatusBar();
    g_nUpdateBits = W_ALL;
}

// ── Grid menu (IDB OnGrid1 0x424ab0 + helpers) ───────────────────────────────────
// uIDCheckItemGrid (IDB 0x6ddf04): the 11 grid-size menu command IDs, parallel to
// grid_sizes (IDB 0x6dde5c = {0.5,1,2,4,8,16,32,64,128,256,512}). Clicking "Grid N"
// sets g_qeglobals.d_gridsize to the matching index.
extern float grid_sizes[];   // engine_stubs.cpp (0x6dde5c)
static const UINT s_gridMenuIDs[11] =
    { 35021, 35022, 35023, 35024, 35025, 35026, 35027, 35029, 35031, 35032, 35033 };

// CheckGridMenuSelection (IDB 0x428950): radio-check the active grid-size menu item.
static void Radiant_CheckGridMenu( CMainFrame *frame )
{
    HMENU m = ::GetMenu( frame->m_hWnd );
    if ( !m )
        return;
    for ( int i = 0; i < 11; ++i )
        ::CheckMenuItem( m, s_gridMenuIDs[i],
            ( i == g_qeglobals.d_gridsize ) ? MF_CHECKED : MF_UNCHECKED );
}

extern "C" int ClampGridSize();   // drag.cpp (0x463a80 — rotate/grid snap table)

// SetGridStatus (IDB 0x428a00) — the full "G:%.1f T:%i R:%i C:%i L:%c%c" status line.
// Now that g_PrefsDlg is the real settings object the texture/rotate-lock + cubic-scale
// fields are read directly (no more grid-size-only fallback): G=grid size,
// T=saved grid index, R=ClampGridSize, C=CubicScale, L=texLock('M')+rotLock('R').
void CMainFrame::SetGridStatus()
{
    const char texLockC = g_PrefsDlg->m_bTextureLock ? 'M' : ' ';
    const char rotLockC = g_PrefsDlg->m_bRotateLock  ? 'R' : ' ';
    char buf[64];
    // NB: d_savedinfo.d_gridsize is a float in the port's qe3.h; the binary passes it
    // to %i, so cast to int explicitly (a float in a %i vararg would misalign the rest).
    _snprintf( buf, sizeof( buf ), "G:%.1f T:%i R:%i C:%i L:%c%c",
               grid_sizes[g_qeglobals.d_gridsize],
               (int)g_qeglobals.d_savedinfo.d_gridsize,
               ClampGridSize(),
               g_PrefsDlg->m_nCubicScale,
               texLockC, rotLockC );
    MainFrm_SetStatusText( 4, buf );
}

// ── Prefs command handlers ───────────────────────────────────────
// A small CheckMenuItem helper (the binary inlines GetMenu→FromHandle→CheckMenuItem
// in each toggle); MF_CHECKED=8, MF_UNCHECKED=0.
static void Radiant_CheckMenu( CMainFrame *frame, UINT id, bool checked )
{
    HMENU m = ::GetMenu( frame->GetSafeHwnd() );
    if ( m )
        ::CheckMenuItem( m, id, checked ? MF_CHECKED : MF_UNCHECKED );
}

extern void PMESH_49();              // pmesh.cpp (0x4495C0) — rebuild all active curveDefs

// OnPrefs (IDB 0x426950) — Edit→Preferences. Opens the dialog (refreshes from the
// registry, edits, saves on OK), applies the settings that can change live, re-checks
// the Snap-to-grid menu item to match the (possibly changed) m_bNoClamp, and broadcasts
// a repaint.  The view-restart prompt + texture-bar re-apply were parked while the
// texture bar was unported; CTextureBar shipped, so both are RESTORED (2026-07-31).
// CTexWnd::UpdatePrefs (0x45D9F0) and the PMESH_49 (0x4495C0) tail are now ported too.
void CMainFrame::OnPrefs()
{
    // 0x426956: both captured BEFORE the dialog re-loads and edits the prefs.
    const int oldTextureBar = g_PrefsDlg->m_bTextureBar;
    const int oldView       = g_PrefsDlg->m_nView;

    if ( Prefs_ShowDialog( this ) == IDOK )
    {
        // 0x42698f: the QE4-window-style change only takes effect on a restart.
        if ( g_PrefsDlg->m_nView != oldView )
            MessageBoxA( "You will need to restart CoD4Radiant for the view changes to take place.",
                         "Radiant", MB_ICONINFORMATION );

        // 0x4269a4: re-apply the texture-browser prefs (search box + scrollbar).
        if ( m_pTexWnd )
            m_pTexWnd->UpdatePrefs();

        // 0x4269bf: re-apply the texture-bar toggle. FAITHFUL BUG — the binary branches
        // on the OLD value (`test ebx,ebx` at 0x4269c1, ebx = the pre-dialog setting),
        // so inside the "it changed" arm it restores the PREVIOUS visibility instead of
        // applying the new one. Transcribed as-is; do not "fix" without a ruling.
        if ( oldTextureBar != g_PrefsDlg->m_bTextureBar )
        {
            ShowControlBar( &m_wndTextureBar, oldTextureBar ? TRUE : FALSE, TRUE );
            ::InvalidateRect( m_wndTextureBar.GetSafeHwnd(), nullptr, TRUE );
        }

        Radiant_CheckMenu( this, 32793, g_PrefsDlg->m_bNoClamp == 0 ); // snap = !NoClamp
        SetGridStatus();
        // 0x426a2d: the curveDef render meshes are tessellated for current_edit_layer,
        // which the prefs dialog can change — regenerate them all.
        PMESH_49();
        g_nUpdateBits = -1;
    }
}

// OnSnaptogrid (IDB 0x428380) — toggle m_bNoClamp, persist, re-check the menu. This is
// the interactive persistence demo: toggling snap flips the grid-snap behaviour
// (drag.cpp / xywnd.cpp / Brush_SnapPlanepts all read m_bNoClamp) and SavePrefs writes
// it to the registry so it survives a restart.
void CMainFrame::OnSnaptogrid()
{
    g_PrefsDlg->m_bNoClamp ^= 1;
    Prefs_SavePrefs( g_PrefsDlg );
    Radiant_CheckMenu( this, 32793, g_PrefsDlg->m_bNoClamp == 0 );     // snap = !NoClamp
    SetGridStatus();
}

// Texture-Lock toggles (IDB OnToggleLockMoves 0x426b80 / OnToggleLockRotations
// 0x429230 / OnToggleLockLightmap 0x426bf0): flip the lock field, re-check the menu,
// SavePrefs, refresh the grid-status line (which shows the lock chars). The lock
// reprojection itself is still deferred in Brush_Move (see brush.cpp), so these
// currently only persist + display the flags.
void CMainFrame::OnToggleLockMoves()
{
    g_PrefsDlg->m_bTextureLock = ( g_PrefsDlg->m_bTextureLock == 0 );
    Radiant_CheckMenu( this, 32785, g_PrefsDlg->m_bTextureLock != 0 );
    Prefs_SavePrefs( g_PrefsDlg );
    SetGridStatus();
}
void CMainFrame::OnToggleLockRotations()
{
    g_PrefsDlg->m_bRotateLock = ( g_PrefsDlg->m_bRotateLock == 0 );
    Radiant_CheckMenu( this, 32835, g_PrefsDlg->m_bRotateLock != 0 );
    Prefs_SavePrefs( g_PrefsDlg );
    SetGridStatus();
}
void CMainFrame::OnToggleLockLightmap()
{
    g_PrefsDlg->m_bLightmapLock = ( g_PrefsDlg->m_bLightmapLock == 0 );
    Radiant_CheckMenu( this, 33237, g_PrefsDlg->m_bLightmapLock != 0 );
    Prefs_SavePrefs( g_PrefsDlg );
    SetGridStatus();
}

// ── View→Show overlay toggles (d_xyShowFlags; IDB 0x42ba40 / 0x42bac0 / 0x42bb00 /
// 0x42bbc0 / 0x42bb60 / 0x42bc20) ──
// Each handler flips its bit in d_savedinfo.d_xyShowFlags, re-checks the menu item
// (CHECKED == feature SHOWN == bit CLEAR — the binary's CheckMenuItem value is 8 when the
// bit is being cleared; Reverse Filter is the one INVERTED case), pushes the state into the
// filter window's checkboxes (CFilterWnd::GetSettings) and broadcasts a repaint.
extern void Map_BuildBrushData();    // map.cpp (0x485f00) — rebuilds the display lists

// CFilterWnd::GetSettings (IDB 0x4140f0) — push d_xyShowFlags into the filter pane's six
// BM_SETCHECK checkboxes.  The port's filter pane is the entity window's Filter mode, so
// this is Radiant_RefreshFilterPane (win_ent.cpp); it no-ops when that window is down.
static void Radiant_SyncFilterWnd()
{
    Radiant_RefreshFilterPane();
}

void CMainFrame::OnSelectNames()              // IDB 0x42ba40
{
    bool nowShown = ( ( g_qeglobals.d_savedinfo.d_xyShowFlags ^ 8 ) & 8 ) == 0;
    g_qeglobals.d_savedinfo.d_xyShowFlags ^= 8u;       // 0x8 = names HIDDEN when set
    Radiant_CheckMenu( this, 33971 /*ID_Names*/, nowShown );
    Map_BuildBrushData();                              // binary rebuilds brush data here
    Radiant_SyncFilterWnd();
    g_nUpdateBits |= 2u;                               // W_XY
}

void CMainFrame::OnSelectCoordinates()        // IDB 0x42bb60
{
    bool nowShown = ( ( g_qeglobals.d_savedinfo.d_xyShowFlags ^ 0x20 ) & 0x20 ) == 0;
    g_qeglobals.d_savedinfo.d_xyShowFlags ^= 0x20u;    // 0x20 = coordinate rulers HIDDEN when set
    Radiant_CheckMenu( this, 33975 /*ID_Coordinates*/, nowShown );
    Radiant_SyncFilterWnd();
    g_nUpdateBits |= 2u;                               // W_XY
}

void CMainFrame::OnSelectReverseFilter()      // IDB 0x42bc20
{
    // NB the menu-check polarity is INVERTED vs the other toggles in the binary
    // (v5=0 when the bit is being cleared, v5=8 when set) — i.e. CHECKED == reverse ON
    // == bit SET.  Transcribed verbatim.
    bool nowOn = ( ( g_qeglobals.d_savedinfo.d_xyShowFlags ^ 0x40 ) & 0x40 ) != 0;
    g_qeglobals.d_savedinfo.d_xyShowFlags ^= 0x40u;    // 0x40 = reverse-filter mode
    Radiant_CheckMenu( this, 36127 /*ID_ReverseFilter*/, nowOn );
    Radiant_SyncFilterWnd();
    ++g_qeglobals.g_filtersUpdated;                    // invalidate FilterBrush's per-brush cache
    if ( g_qeglobals.d_hwndCamera )
        ::SetFocus( g_qeglobals.d_hwndCamera );
    g_nUpdateBits = -1;                                // full repaint (filter visibility changed)
}

void CMainFrame::OnSelectConnections()        // IDB 0x42bbc0
{
    // d_xyShowFlags bit 0x4 = connection lines HIDDEN when SET (shown when clear).  Draw
    // consumers = Lines_AddLinkTo + Lines_AddLinkToScript (xywnd.cpp), both ported (the
    // connections unit) and invoked from OnPaint/Cam_Draw via Ed_DrawConnectionLines.
    bool nowShown = ( ( g_qeglobals.d_savedinfo.d_xyShowFlags ^ 4 ) & 4 ) == 0;
    g_qeglobals.d_savedinfo.d_xyShowFlags ^= 4u;
    Radiant_CheckMenu( this, 33974 /*ID_Connections*/, nowShown );
    Radiant_SyncFilterWnd();                           // mirror into the filter window's checkbox
    g_nUpdateBits |= 0xBu;                             // W_CAMERA|W_XY|W_Z (binary: |= 0xB)
}

void CMainFrame::OnSelectAngles()             // IDB 0x42bac0
{
    // d_xyShowFlags bit 0x2 = entity angle arrows HIDDEN when SET (shown when clear).  Draw
    // consumer = DrawAngles (brush.cpp), invoked from DrawBrush for fixedsize "angles"-keyed
    // entities in BOTH the XY views and the camera — hence g_nUpdateBits |= 3 (W_XY|W_CAMERA).
    bool nowShown = ( ( g_qeglobals.d_savedinfo.d_xyShowFlags ^ 2 ) & 2 ) == 0;
    g_qeglobals.d_savedinfo.d_xyShowFlags ^= 2u;
    Radiant_CheckMenu( this, 33972 /*ID_Angles*/, nowShown );
    Radiant_SyncFilterWnd();
    g_nUpdateBits |= 3u;                               // W_XY|W_CAMERA (binary: |= 3)
}

void CMainFrame::OnSelectBlocks()             // IDB 0x42bb00
{
    // d_xyShowFlags bit 0x10 = the 1024-unit block grid HIDDEN when SET (shown when clear).
    // Draw consumer = XY_DrawBlockGrid (xywnd.cpp), invoked from OnPaint when the bit is clear.
    bool nowShown = ( ( g_qeglobals.d_savedinfo.d_xyShowFlags ^ 0x10 ) & 0x10 ) == 0;
    g_qeglobals.d_savedinfo.d_xyShowFlags ^= 0x10u;
    Radiant_CheckMenu( this, 33973 /*ID_Blocks*/, nowShown );
    Radiant_SyncFilterWnd();
    g_nUpdateBits |= 2u;                               // W_XY (block grid is XY-only)
}

// OnTexturesInspector (IDB 0x424b60) — Textures→Surface Inspector → DoSurface.  Opens (or
// re-focuses) the hand-built Surface Inspector popup; selecting a face then populates it,
// and edits re-project the texture live in the camera (surfacedlg.cpp).
void CMainFrame::OnTexturesInspector()
{
    Surf_OpenInspector();
}

// OnPatchInspector (IDB 0x42b460, cmd 33092, Shift+S) — Patch→Inspector → DoPatchInspector.
// Opens (or re-focuses) the hand-built Patch Properties popup and populates it from the
// selected patch (patchdialog.cpp).
extern void DoPatchInspector();   // patchdialog.cpp (0x436d30)
void CMainFrame::OnPatchInspector()
{
    DoPatchInspector();
}

// OnCurveSimplepatchmesh (IDB 0x429a20, cmd 32856 / 0x8058 — verified from the CMainFrame
// command table {cmdId,cmdId,0x38,pfn}) — Curve→Simple Patch Mesh.  Runs the "Patch
// density" modal (CPatchDensityDlg) inside an Undo bracket; OK builds a flat NxM Bezier
// patch over the selected brush (patchdialog.cpp → Patch_GenericMesh).
extern void DoSimplePatchMesh();   // patchdialog.cpp (0x429a20 body)
void CMainFrame::OnCurveSimplepatchmesh()
{
    DoSimplePatchMesh();
}

extern void DoSimpleTerrainPatchMesh( bool terrain );   // patchdialog.cpp
void CMainFrame::OnCurveSimpleterrainpatch()
{
    DoSimpleTerrainPatchMesh( true );
}

// OnTextureReplaceall (IDB 0x428b40, a thunk) — Textures→Replace All → open/show the
// Find/Replace Texture(s) dialog (findtexture.cpp).
void CMainFrame::OnTextureReplaceall()
{
    CFindTextureDlg::show();
}

// OnLayersDlg (IDB 0x42bd10) — toggle the Layers management dialog (layersdlg.cpp).
// The binary ShowWindow(SW_HIDE/SW_SHOW)s a global LayersDlg + g_nUpdateBits|=1;
// CLayerDlg::Toggle reproduces that (and lazily creates the dialog on first use).
void CMainFrame::OnLayersDlg()
{
    CLayerDlg::Toggle();
}

// OnSelectionAddToActiveLayer (IDB CXYWnd::OnSelectionAddToActiveLayer 0x466930,
// AFX msgmap nID 0x88B9 = 35001) — the right-click "Add selection to active layer"
// command.  The binary attaches it to the CXYWnd context-menu map; KisakCOD routes
// menu commands through CMainFrame, so this thin handler forwards to the ported
// core (brush.cpp), which sets every selected brush's parent_layer_string to
// g_activeLayer_string.  g_nUpdateBits|=1 → redraw (CMainFrame::On* convention).
extern void CXYWnd_OnSelectionAddToActiveLayer();   // brush.cpp (0x466930 core)
void CMainFrame::OnSelectionAddToActiveLayer()
{
    CXYWnd_OnSelectionAddToActiveLayer();
    g_nUpdateBits |= 1;
}

// OnMiscDynEntities (IDB 0x42bd90) — toggle the Dyn-Entity authoring dialog
// (dynentitydlg.cpp).  The binary ShowWindow(SW_HIDE/SW_SHOW)s the global DynEntityDlg
// + g_nUpdateBits|=1; CDynEntityDlg::Toggle reproduces that (lazy-create on first use).
void CMainFrame::OnMiscDynEntities()
{
    CDynEntityDlg::Toggle();
}

// OnMiscVehicleGroup (IDB 0x42bd50) — toggle the Vehicle-Group authoring dialog
// (vehicledlg.cpp).  The binary ShowWindow(SW_HIDE/SW_SHOW)s the global VehicleDlg
// (and hides the entity window when showing) + g_nUpdateBits|=1; CVehicleDlg::Toggle
// reproduces that (lazy-create on first use).
void CMainFrame::OnMiscVehicleGroup()
{
    CVehicleDlg::Toggle();
}

// OnReplaceModels (IDB 0x42bf00) — toggle the Replace-Models tool dialog (modeldlg.cpp).
// The binary ShowWindow(SW_HIDE/SW_SHOW)s the global ModelDlg + g_nUpdateBits|=1;
// CModelDlg::Toggle reproduces that (lazy-create on first use).
void CMainFrame::OnReplaceModels()
{
    CModelDlg::Toggle();
}

// OnVertexEditDlg (IDB 0x42bcd0) — toggle the patch Vertex-Edit dialog (verteditdlg.cpp).
// The binary ShowWindow(SW_HIDE/SW_SHOW)s the global VertEditDlg + g_nUpdateBits|=1;
// CVertEditDlg::Toggle reproduces that (lazy-create on first use).  Command 33199 (accel 'G').
void CMainFrame::OnVertexEditDlg()
{
    CVertEditDlg::Toggle();
}

// OnEditMapinfo (IDB 0x426c60) — Edit→Map Info... open the read-only Map Info dialog
// (mapinfo.cpp).  The binary constructs a CMapInfo on the stack and DoModal()s it; with
// no .rc template the dialog is hand-built, so CMapInfo::Show (re)creates + populates +
// shows the popup against the current map (recompute-per-open, faithful to the binary's
// fresh-dialog-per-invoke).
void CMainFrame::OnEditMapinfo()
{
    CMapInfo::Show();
}

// OnEditEntityinfo (IDB 0x426D6F) — Edit→Entity Info... open the Entity List browser
// (entitylist.cpp).  The binary constructs a CEntityListDlg on the stack and DoModal()s
// it; with no .rc template the dialog is hand-built, so EntList_Open (re)creates +
// populates + shows the popup against the current map (a fresh entity-list browse per
// invoke, matching the binary's per-DoModal rebuild in InsertItems/OnInitDialog).
void CMainFrame::OnEditEntityinfo()
{
    EntList_Open();
}

// OnGrid1 (IDB 0x424ab0): set the grid size from the clicked menu item, refresh the
// menu radio check + status, redraw the XY/Z views. ON_COMMAND_EX passes the command ID.
BOOL CMainFrame::OnGridSize( UINT nID )
{
    for ( int i = 0; i < 11; ++i )
        if ( nID == s_gridMenuIDs[i] ) { g_qeglobals.d_gridsize = i; break; }
    Radiant_CheckGridMenu( this );
    SetGridStatus();
    extern void Sys_UpdateWindows( int bits );   // win_qe3.cpp
    Sys_UpdateWindows( W_XY | W_Z );
    return TRUE;
}

// ════════════════════════════════════════════════════════════════════════════
// View + Selection menu wrappers - thin command handlers forwarding to ported cores.

extern selbrush_t selected_brushes;                      // engine_stubs (0x23F1864)
extern void    Undo_ClearRedo();                         // undo.cpp
extern void    Undo_GeneralStart( const char *operation );
extern void    Undo_AddBrushList( selbrush_t *sb );
extern void    Undo_EndBrushList( selbrush_t *sb );
extern void    Undo_End();
extern void    Select_Deselect( int bDeselectFaces );    // select.cpp (0x48E800)
extern void    CSG_MakeHollow();                          // csg.cpp    (0x47D3C0)
extern int     CSG_Merge();                               // csg.cpp    (0x47DA40)
extern void    Sys_UpdateWindows( int bits );             // win_qe3.cpp
extern int     Sys_Printf( const char *fmt, ... );        // win_qe3.cpp (0x499E90)
extern float   z_scale;                                   // z.cpp       (0x241A5B0)

// IDB global 0x25D5A90 — last committed XY zoom level (pixels/world unit). The zoom
// handlers mirror m_fScale into it; the overlay/print paths read it.
float g_zoomLevel = 1.0f;

// CXYWnd::OnViewZoomin (0x424750): zoom the active XY view in 1.25×, clamp to 160,
// and zoom the Z view to match. (View→Zoom→XY Zoom In, ID 32995 / Delete.)
void CMainFrame::OnViewZoomin()
{
    if ( m_pXYWnd && m_pXYWnd->m_bActive )
    {
        m_pXYWnd->m_fScale *= 1.25f;
        if ( m_pXYWnd->m_fScale > 160.0f )
            m_pXYWnd->m_fScale = 160.0f;
        g_zoomLevel = m_pXYWnd->m_fScale;
    }
    Sys_UpdateWindows( W_XY | W_XY_OVERLAY | W_Z | W_Z_OVERLAY );
    z_scale *= 1.25f;
    if ( z_scale > 160.0f )
        z_scale = 160.0f;
}

// CXYWnd::OnViewZoomout (0x4247e0): zoom the active XY view out 0.8×, clamp to
// 0.003125, Z view follows. (View→Zoom→XY Zoom Out, ID 32996 / Insert.)
void CMainFrame::OnViewZoomout()
{
    if ( m_pXYWnd && m_pXYWnd->m_bActive )
    {
        float s = m_pXYWnd->m_fScale * 0.800000011920929f;
        if ( s < 0.003125000046566129f )
            s = 0.003125f;
        m_pXYWnd->m_fScale = s;
        g_zoomLevel = s;
    }
    z_scale *= 0.800000011920929f;
    if ( z_scale < 0.003125000046566129f )
        z_scale = 0.003125f;
    Sys_UpdateWindows( W_XY | W_XY_OVERLAY | W_Z | W_Z_OVERLAY );
}

// CMainFrame::OnView100 (0x423c30): reset the XY view to 1:1. (View→Zoom→XY 100%,
// ID 32968.)
void CMainFrame::OnView100()
{
    if ( m_pXYWnd )
        m_pXYWnd->m_fScale = 1.0f;
    Sys_UpdateWindows( W_XY | W_XY_OVERLAY );
}

// CMainFrame::OnSelectionDeselect (0x425740): deselect every selected brush + face.
// (Selection->Deselect, ID 33002 / Esc.)  The surf-inspector / clip / rotate / scale /
// curve-point branches only fire when that mode is engaged; the binary otherwise falls
// through to Select_Deselect.
void CMainFrame::OnSelectionDeselect()
{
    Select_Deselect( 1 );
    MainFrm_SetStatusText( 2, " " );
}

// ── VERTEX-EDIT toggle (Selection→Drag Vertices, ID 33005) ───────────────────
// CMainFrame::OnSelectionDragVertices (0x425840): sel_vertex/sel_curvepoint → back to
// sel_brush; otherwise route pure-patch selections into patch/terrain edit, else build the
// vertex handle list (SetupVertexSelection) and enter sel_vertex if there are any handles.
// The tail cleans up whichever patch sub-mode was active BEFORE the switch.
extern int  OnlyPatchesSelected();   // engine_stubs.cpp 0x447860
extern int  AnyPatchesSelected();    // engine_stubs.cpp 0x447890
extern void SetupVertexSelection();  // select.cpp 0x494bc0
extern void Patch_EditPatch();       // engine_stubs.cpp (patch editor, FATAL until pmesh)
extern void Terrain_Edit();          // pmesh.cpp (mixed patch+brush vert-snap entry, 0x442100)
extern void sub_43ECB0();            // engine_stubs.cpp (addpoint-mode cleanup, FATAL)
extern void CMainFrame_UpdatePatchToolbarButtons();   // select.cpp (0x42AA70)

void CMainFrame::OnSelectionDragVertices()
{
    select_t prevMode = g_qeglobals.d_select_mode;
    if ( prevMode == sel_vertex || prevMode == sel_curvepoint )
    {
        g_qeglobals.d_select_mode = sel_brush;   // toggle OFF
    }
    else
    {
        if ( OnlyPatchesSelected() )             // pure-patch → patch vertex edit
        {
            Patch_EditPatch();
            g_nUpdateBits = -1;
            return;
        }
        if ( AnyPatchesSelected() )              // mixed-with-patch → terrain edit
        {
            Terrain_Edit();
            g_nUpdateBits = -1;
            return;
        }
        SetupVertexSelection();                  // build the point/edge handle lists
        if ( !g_qeglobals.d_numpoints )          // nothing to drag → bail
        {
            g_nUpdateBits = -1;
            return;
        }
        // The binary RE-READS d_select_mode here (0x425898) — SetupVertexSelection runs first.
        prevMode = g_qeglobals.d_select_mode;
        g_qeglobals.d_select_mode = sel_vertex;  // toggle ON
    }

    // Clean up whichever patch sub-mode was active before the switch.
    if ( prevMode == sel_cycle_edge_direction_quad )
    {
        CMainFrame_UpdatePatchToolbarButtons();
        g_nUpdateBits = -1;
        return;
    }
    if ( prevMode == sel_addpoint )
    {
        sub_43ECB0();
    }
    g_nUpdateBits = -1;
}

// ── EDGE-EDIT toggle (Selection→Drag Edges, ID 33006) ────────────────────────
// CMainFrame::OnSelectionDragedges (0x4257d0): the sibling of the vertex toggle,
// but simpler — it does NOT route pure-patch selections (no OnlyPatchesSelected /
// AnyPatchesSelected branch). sel_edge → back to sel_brush; otherwise build the
// handle lists (SetupVertexSelection) and enter sel_edge if there are any points.
void CMainFrame::OnSelectionDragEdges()
{
    if ( g_qeglobals.d_select_mode == sel_edge )
    {
        g_qeglobals.d_select_mode = sel_brush;   // toggle OFF
        g_nUpdateBits = -1;
        return;
    }
    SetupVertexSelection();                       // build the point/edge handle lists
    if ( g_qeglobals.d_numpoints )
    {
        select_t prevMode = g_qeglobals.d_select_mode;
        g_qeglobals.d_select_mode = sel_edge;     // toggle ON
        if ( prevMode == sel_cycle_edge_direction_quad )
        {
            CMainFrame_UpdatePatchToolbarButtons();
            g_nUpdateBits = -1;
            return;
        }
        if ( prevMode == sel_addpoint )
            sub_43ECB0();
    }
    g_nUpdateBits = -1;
}

// ── DROP SELECTION TO FLOOR (Selection→Drop to Floor, ID 33183) ──────────────
// CMainFrame::OnDropSelected (0x425be0): for each selected brush/entity, trace a
// ray straight DOWN from its origin (fixed-size point entities start +16 up) and
// move it to the first surface below. Fixed-size entities optionally orient to the
// floor normal (OrientModel) and get a random pitch/roll/yaw + modelscale scatter
// (DropModel). The drop is raised by DropHeight unless ForceZeroDropHeight; non-
// fixed-size brushes snap to the grid unless NoClamp. The whole pass is one undo
// bracket, and (like the binary) prefs are reloaded from the registry at entry.
//   The grid-snap loop's apparent IDB off-by-one (reads pos@+0x54.., writes @+0x50..) is a
//   stack-shift display artifact, not a shifted write; both arrays are the same pos[3].
extern void          sub_47CBA0( selbrush_t *b, int axis, float deg );                       // select.cpp (0x47CBA0)
extern char         *va( const char *fmt, ... );                                             // q_shared
extern edTrace_t    *Trace_AllDirectionsIfFailed( float *cam_origin, edTrace_t *trace_result,
                                                  float *dir, int contents );                // select.cpp (0x48DAA0)
extern void          AlignEntityToFace_OrientToFloor( entity_s_def *ent, float *dir );        // entity.cpp (0x485AD0)
extern entity_s     *Brush_Move( const float *move, brush_t *def, char snap );               // brush.cpp (0x47BA40)

void CMainFrame::OnDropSelected()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "drop selection" );
    Undo_AddBrushList( &selected_brushes );
    Prefs_LoadPrefs( g_PrefsDlg );                       // binary: CPrefsDlg::LoadPrefs (0x44e330)

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        // Cancel any in-progress patch sub-mode and force plain brush-select.
        select_t prevMode = g_qeglobals.d_select_mode;
        g_qeglobals.d_select_mode = sel_brush;
        if ( prevMode == sel_cycle_edge_direction_quad )
        {
            CMainFrame_UpdatePatchToolbarButtons();
        }
        else if ( prevMode == sel_addpoint )
        {
            sub_43ECB0();                                // Patch_FinishCurveDrag (0x43ecb0)
        }

        entity_s_def *def = (entity_s_def *)b->owner->def;

        // ── random model scatter (fixed-size entities, when DropModel set) ──
        if ( *(int *)&def->eclass->fixedsize && g_PrefsDlg->m_bDropModel )
        {
            SetKeyValue( def, "angles", "0 0 0" );
            sub_47CBA0( b, 0, (float)( (double)rand() * 0.000030517578125 ) * 5.0f - 2.5f );   // pitch ±2.5
            sub_47CBA0( b, 1, (float)( (double)rand() * 0.000030517578125 ) * 5.0f - 2.5f );   // roll  ±2.5
            sub_47CBA0( b, 2, (float)( (double)rand() * 0.000030517578125 ) * 360.0f );        // yaw   0..360
            float base  = (float)g_PrefsDlg->scale_base  / 100.0f;
            float range = (float)g_PrefsDlg->scale_range / 100.0f;
            float scale = (float)( (double)rand() * 0.000030517578125 ) * ( range + range ) + base - range;
            SetKeyValue( def, "modelscale", va( "%f", scale ) );
        }

        // ── trace straight down from the entity origin (fixed-size starts +16 up) ──
        float dir[3]        = { 0.0f, 0.0f, -1.0f };
        float cam_origin[3] = { def->origin[0], def->origin[1], def->origin[2] };
        if ( *(int *)&def->eclass->fixedsize )
            cam_origin[2] += 16.0f;

        edTrace_t tr;
        Trace_AllDirectionsIfFailed( cam_origin, &tr, dir, 4610 );
        if ( !tr.hit.brush )
            continue;

        if ( *(int *)&def->eclass->fixedsize && g_PrefsDlg->m_bOrientModel )
            AlignEntityToFace_OrientToFloor( def, tr.normal );

        float pos[3];
        pos[0] = dir[0] * tr.dist + cam_origin[0];
        pos[1] = dir[1] * tr.dist + cam_origin[1];
        pos[2] = dir[2] * tr.dist + cam_origin[2];
        if ( !g_PrefsDlg->m_bForceZeroDropHeight )
            pos[2] += (float)g_PrefsDlg->m_dropHeight;

        // Non-fixed-size brushes snap to the grid (unless NoClamp).
        if ( !*(int *)&def->eclass->fixedsize && !g_PrefsDlg->m_bNoClamp )
        {
            float gs = grid_sizes[g_qeglobals.d_gridsize];
            for ( int i = 0; i < 3; ++i )
                pos[i] = (float)floor( pos[i] / gs + 0.5 ) * gs;
        }

        float move_delta[3];
        move_delta[0] = pos[0] - def->origin[0];
        move_delta[1] = pos[1] - def->origin[1];
        move_delta[2] = pos[2] - def->origin[2];
        Brush_Move( move_delta, b->def, 0 );
    }

    g_nUpdateBits = -1;
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// CMainFrame::OnSelectionMakehollow (0x425570): undo-wrapped CSG hollow of the one
// selected brush. (Selection→CSG→Hollow, ID 32982.) Refuses >1 brush like the binary.
void CMainFrame::OnSelectionMakehollow()
{
    // Count the selection: the binary walks at most one extra node, then errors.
    selbrush_t *b = selected_brushes.next;
    int extra = 0;
    while ( b != &selected_brushes && ++extra <= 1 )
        b = b->next;
    if ( b != &selected_brushes )
    {
        Sys_Printf( "Can't hollow more than 1 brush at a time.\n" );
        return;
    }
    Undo_ClearRedo();
    Undo_GeneralStart( "hollow" );
    Undo_AddBrushList( &selected_brushes );
    CSG_MakeHollow();
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// CMainFrame::OnSelectionCsgmerge (0x4255d0): undo-wrapped CSG merge of the
// selection into one convex brush. (Selection→CSG→Merge, ID 32927.)
void CMainFrame::OnSelectionCsgmerge()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "CSG merge" );
    Undo_AddBrushList( &selected_brushes );
    CSG_Merge();
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// ── CLIPPER command handlers ─────────────────────────────────────────────────
// The clipper cluster (xywnd.cpp). OnViewClipper toggles clip mode (cancelling mouse-rotate
// mode on the way IN) and check-marks the toolbar button; OnClipSelected commits the kept
// side (Enter), undo-bracketed; OnFlipClip swaps which side the clip keeps (X).
extern void Ed_SetClipMode( char bMode );    // xywnd.cpp (0x465430)
extern void Ed_Clip( CXYWnd *wnd );           // xywnd.cpp (0x46dc10)
extern void Ed_FlipClip();                    // xywnd.cpp (0x46ddf0)
extern int  g_bClipMode;                      // engine_stubs.cpp 0x23f16d8
extern bool g_bRotateMode;                    // drag.cpp (0x23F16D9)
extern int  g_bPatchBendMode;                 // pmesh.cpp (0x25D5B04)
void Ed_InvalidateAllViews();                 // defined below (repaint broadcast)

void CMainFrame::OnViewClipper()              // 0x426510
{
    if ( !m_pActiveXY )
        return;
    if ( g_bClipMode )
    {
        Ed_SetClipMode( 0 );
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32783, FALSE );
    }
    else
    {
        if ( g_bRotateMode )                  // entering clip mode cancels mouse-rotate first
            OnSelectMouserotate();
        Ed_SetClipMode( 1 );
        m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32783, TRUE );
    }
    Ed_InvalidateAllViews();                  // KISAK: the binary repaints from SetClipMode's bits
}

extern void Patch_BendHandleEnter();          // pmesh.cpp (0x447B70)

void CMainFrame::OnClipSelected()             // 0x427170
{
    if ( m_pActiveXY && g_bClipMode )
    {
        Undo_ClearRedo();
        Undo_GeneralStart( "clip selected" );
        Undo_AddBrushList( &selected_brushes );
        Ed_Clip( m_pActiveXY );
        Undo_EndBrushList( &selected_brushes );
        Undo_End();
        Ed_InvalidateAllViews();
    }
    else if ( g_bPatchBendMode )
    {
        Patch_BendHandleEnter();   // 0x447b70 — advance the bend state machine
    }
}

void CMainFrame::OnFlipClip()                 // 0x427140
{
    if ( m_pActiveXY )
    {
        Ed_FlipClip();
        Ed_InvalidateAllViews();
    }
}

// Region menu wrappers - 5-byte jmp thunks in the IDB (0x4252b0..0x4252f0) to the map.cpp
// cores; no undo wrapper (a selection change, not an edit).
extern void Map_RegionOff();              // map.cpp (0x487530)
extern void Map_RegionXY();               // map.cpp (0x4877d0)
extern void Map_RegionTallBrush();        // map.cpp (0x487860)
extern void Map_RegionBrush();            // map.cpp (0x4878e0)
extern void Map_RegionSelectedBrushes();  // map.cpp (0x487720)

void CMainFrame::OnRegionOff()          { Map_RegionOff(); }            // 0x4252b0
void CMainFrame::OnRegionSetxy()        { Map_RegionXY(); }            // 0x4252f0
void CMainFrame::OnRegionSettallbrush() { Map_RegionTallBrush(); }     // 0x4252e0
void CMainFrame::OnRegionSetbrush()     { Map_RegionBrush(); }         // 0x4252c0
void CMainFrame::OnRegionSetselection() { Map_RegionSelectedBrushes(); } // 0x4252d0

// ── BRUSH → PRIMITIVES command handlers ──────────────────────────────────────
// Reshape the selected brush into an N-sided cylinder / cone / sphere.  The binary pops the
// IDD_ARBITRARY_SIDES modal (SidesDlgProc 0x495F00), which dispatches to
// Brush_MakeSided{,Cone,Sphere} via the g_bDoCone / g_bDoSphere flags.  All three handlers
// are undo-bracketed (0x424EE0 / 0x429170 / 0x42B630).
extern void Brush_MakeSided_Prolog( unsigned int sides, char snap ); // brush.cpp (0x4735E0)
extern void Brush_MakeSidedCone( int sides );                        // brush.cpp (0x47BC10)
extern void Brush_MakeSidedSphere( int sides );                      // brush.cpp (0x47BE90)

// IDB globals 0x25D5B38 / 0x25D5B39 — which primitive SidesDlgProc builds.
char g_bDoCone   = 0;
char g_bDoSphere = 0;

// SidesDlgProc (0x495F00) — the modal "number of sides" dialog procedure.  Reads
// the IDC_ARB_SIDES_IN edit field on OK and builds the primitive selected by the
// g_bDoCone / g_bDoSphere flags.  Faithful to the binary (atol of the field text).
static INT_PTR CALLBACK SidesDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM )
{
    if ( msg == WM_INITDIALOG )
    {
        ::SetFocus( ::GetDlgItem( hDlg, IDC_ARB_SIDES_IN ) );
        return FALSE;       // we set focus ourselves (return 0 like the binary)
    }
    if ( msg != WM_COMMAND )
        return FALSE;

    WORD id = LOWORD( wParam );
    if ( id == IDCANCEL )
    {
        ::EndDialog( hDlg, 0 );
        return FALSE;
    }
    if ( id != IDOK )
        return FALSE;

    char text[256] = { 0 };
    ::GetWindowTextA( ::GetDlgItem( hDlg, IDC_ARB_SIDES_IN ), text, 255 );
    int sides = atol( text );
    if ( g_bDoCone )
        Brush_MakeSidedCone( sides );
    else if ( g_bDoSphere )
        Brush_MakeSidedSphere( sides );
    else
        Brush_MakeSided_Prolog( (unsigned int)sides, 1 );
    ::EndDialog( hDlg, 1 );
    return FALSE;
}

// Run the modal sides dialog, undo-bracketed (shared by all three handlers).
static void Radiant_RunSidesDialog( const char *undoName, char doCone, char doSphere )
{
    Undo_ClearRedo();
    Undo_GeneralStart( undoName );
    Undo_AddBrushList( &selected_brushes );
    g_bDoCone   = doCone;
    g_bDoSphere = doSphere;
    ::DialogBoxParamA( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDD_ARBITRARY_SIDES ),
                       g_pParentWnd ? g_pParentWnd->m_hWnd : nullptr,
                       SidesDlgProc, 0 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

void CMainFrame::OnBrushArbitrarysided()      // 0x424EE0  (Brush→Arbitrary sided cylinder)
{
    Radiant_RunSidesDialog( "arbitrary sided", 0, 0 );
}

void CMainFrame::OnBrushMakecone()            // 0x429170  (Brush→Primitives→Cone)
{
    Radiant_RunSidesDialog( "make cone", 1, 0 );
}

void CMainFrame::OnBrushPrimitivesSphere()    // 0x42B630  (Brush→Primitives→Sphere)
{
    Radiant_RunSidesDialog( "make sphere", 0, 1 );
}

// ── SELECTION menu wrappers (batch — thin forwards to already-ported cores) ───
// Each transcribed from the IDB; the Make Detail/Structural pair is undo-bracketed exactly
// as the binary.
extern void Select_Connected();                          // select.cpp (0x490EC0)
extern void Select_ByClass( const char *key );           // select.cpp (0x490A70)
extern LRESULT Select_Ungroup();                         // entity.cpp (0x490780)
extern void Select_ChangeBrushType( int contents, int mask ); // select.cpp (0x491790)

void CMainFrame::OnSelectConneted()           // 0x425550 — Selection→Select Connected (33134)
{
    Select_Connected();
}

void CMainFrame::OnSelectionTargetname()      // 0x426390 — Selection→Select Targetname (33132)
{
    Select_ByClass( "targetname" );
}

void CMainFrame::OnSelectionClassname()       // 0x4263A0 — Selection→Select Classname (202)
{
    Select_ByClass( "classname" );
}

void CMainFrame::OnSelectionUngroupentity()   // 0x426380 — Selection→Ungroup entity (33035)
{
    Select_Ungroup();
}

extern void Select_ByKeyValue();                         // select.cpp (0x490C00)

void CMainFrame::OnSelectionKeyValue()        // 0x4263B0 — Selection→Select by Key/Value (33133)
{
    Select_ByKeyValue();
}

void CMainFrame::OnSelectionMakeDetail()      // 0x4261C0 — Selection→Make Detail (33042)
{
    Undo_ClearRedo();
    Undo_GeneralStart( "make detail" );
    Undo_AddBrushList( &selected_brushes );
    Select_ChangeBrushType( 0x8000000, 8320 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

void CMainFrame::OnSelectionMakeStructural()  // 0x426200 — Selection→Make Structural (33043)
{
    Undo_ClearRedo();
    Undo_GeneralStart( "make structural" );
    Undo_AddBrushList( &selected_brushes );
    Select_ChangeBrushType( 0, 134226052 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// ── SCRIPT-GROUP / FIXED-SIZE LIGHT command handlers (0x428E10-0x428EE0) ─────────
// Thin CMainFrame methods the binary jumps through to the brush.cpp/pmesh.cpp cores,
// transcribed verbatim incl. the exact float factors.
extern void DisassociateEntities();          // brush.cpp 0x47a1b0
extern void SelectedAssociated();            // brush.cpp 0x47a330
extern void OverbrightShift( float a1 );     // brush.cpp 0x47a600
extern void Light_01( float a1 );            // brush.cpp 0x47a790
extern void Light_Height( float a1 );        // brush.cpp 0x47a910
extern void Patch_Subdivide( int decrease ); // pmesh.cpp 0x44cde0
// Script-Group colour command cores (scriptgroup.cpp).
extern void ScriptGroup_SyncGroupKeyToTeam();   // 0x454480 (force ScriptGroupKey = ScriptColorTeamKey)
extern void ScriptGroup_AddColorToSelection();  // 0x4543B0
extern void ScriptGroup_TriggerNumber();        // 0x453170
extern bool ScriptGroup_SelectionHasTrigger();  // (helper used by OnScriptGroup_Disassociate)
extern void AssociateEntities();                // 0x455A80 (Script-Group dialog toggle/open)
extern int  UpdateSelection( int wParam, eclass_t *cls );   // win_ent.cpp 0x497180 (declared again @2812)
extern int  Sys_Printf( const char *fmt, ... );             // win_qe3.cpp 0x499E90 (declared again @2021)

// 0x428E00 (thunk)  CMainFrame::OnAssociateEntities — Associate-Entities accelerator (Shift+G):
// open the Script-Group dialog.
void CMainFrame::OnAssociateEntities()
{
    AssociateEntities();
}

// 0x424E20 (thunk)  CMainFrame::OnScriptGroup — the "Script group" menu item (id 200): same
// AssociateEntities core as OnAssociateEntities (a distinct thunk in the binary).
void CMainFrame::OnScriptGroup()
{
    AssociateEntities();
}

void CMainFrame::OnDisassociateEntities()   // 0x428E10 (thunk → DisassociateEntities)
{
    DisassociateEntities();
}

void CMainFrame::OnSelectedAssociated()     // 0x428E20 (thunk → SelectedAssociated)
{
    SelectedAssociated();
}

// 0x4264A0  CMainFrame::OnScriptGroup_01 — "Add Color Group" command: force the colour-team
// key, then run ScriptGroup_AddColorToSelection.
void CMainFrame::OnScriptGroup_01()
{
    ScriptGroup_SyncGroupKeyToTeam();
    ScriptGroup_AddColorToSelection();
}

// 0x426460  CMainFrame::OnScriptGroup_Disassociate — the Script-Group "Disassociate" command:
// force the colour-team key, require a trigger in the selection, then ScriptGroup_TriggerNumber.
void CMainFrame::OnScriptGroup_Disassociate()
{
    ScriptGroup_SyncGroupKeyToTeam();
    if ( !ScriptGroup_SelectionHasTrigger() )
    {
        Sys_Printf( "You must select a trigger_multiple or trigger_radius in combination with the nodes, AI, or goal volumes you with to disassociate.\n" );
        return;
    }
    ScriptGroup_TriggerNumber();
    UpdateSelection( 0xFFFFFFFF, 0 );
    g_nUpdateBits = -1;
}

void CMainFrame::OnLightShiftUp()           // 0x428E30
{
    Light_01( 1.1f );
    g_nUpdateBits = -1;
}

void CMainFrame::OnLightShiftDown()         // 0x428E50
{
    Light_01( 0.9f );
    g_nUpdateBits = -1;
}

void CMainFrame::OnCyclinderHeightUp()      // 0x428E70
{
    Light_Height( 1.1f );
    g_nUpdateBits = -1;
}

void CMainFrame::OnCyclinderHeightDown()    // 0x428E90
{
    Light_Height( 0.9f );
    g_nUpdateBits = -1;
}

// ── Camera raise/lower (D / C) — IDB OnCameraUp 0x426900 / OnCameraDown 0x426680 ──
//   Nudge the 3D camera up/down by 32 world units along Z, then redraw the camera (W_CAMERA)
//   and, if "camera updates XY" is on, the 2D view (W_XY).  Faithful to the binary's
//   `g_nUpdateBits |= 2*(m_bCamXYUpdate!=0)+1`.
void CMainFrame::OnCameraUp()               // 0x426900 (cmd 33055, 'D')
{
    m_pCamWnd->camera.origin[2] += 32.0f;
    g_nUpdateBits |= 2 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
}

void CMainFrame::OnCameraDown()             // 0x426680 (cmd 33056, 'C')
{
    m_pCamWnd->camera.origin[2] -= 32.0f;
    g_nUpdateBits |= 2 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
}

// ── Camera pitch up/down (A / Z) — IDB OnCameraAngleUp 0x4265d0 / OnCameraAngleDown 0x426590 ──
//   Tilt the 3D camera pitch (angles[0]) by ±22.5°, clamped to [-85, 85], then redraw the
//   camera (W_CAMERA).  Faithful to the binary.
void CMainFrame::OnCameraAngleUp()          // 0x4265d0 (cmd 33061, 'A')
{
    g_nUpdateBits |= W_CAMERA;
    m_pCamWnd->camera.angles[0] += 22.5f;
    if ( m_pCamWnd->camera.angles[0] > 85.0f )
        m_pCamWnd->camera.angles[0] = 85.0f;
}

void CMainFrame::OnCameraAngleDown()        // 0x426590 (cmd 33062, 'Z')
{
    g_nUpdateBits |= W_CAMERA;
    m_pCamWnd->camera.angles[0] -= 22.5f;
    if ( m_pCamWnd->camera.angles[0] < -85.0f )
        m_pCamWnd->camera.angles[0] = -85.0f;
}

void CMainFrame::OnOverBrightShiftUp()      // 0x428EB0
{
    OverbrightShift( -0.05f );
    Patch_Subdivide( -1 );
    g_nUpdateBits = -1;
}

void CMainFrame::OnOverBrightShiftDown()    // 0x428EE0
{
    OverbrightShift( 0.05f );
    Patch_Subdivide( 1 );
    g_nUpdateBits = -1;
}

// ── CMainFrame::Nudge (0x4298B0) — nudge the selection by fNudge along axis nDim ─────
// In curve/terrain-point edit mode with picked move-points it drives MoveSelection
// (curve-point translate); otherwise Select_Move (whole-brush translate, grid-snapped).
void CMainFrame::Nudge( int nDim, float fNudge )
{
    extern void MoveSelection( float *origin, float *dir, float *move );  // drag.cpp
    extern void Select_Move( const float *delta, char bSnap );            // select.cpp 0x48E9C0
    float a[3] = { 0.0f, 0.0f, 0.0f };
    a[nDim] = fNudge;
    if ( ( g_qeglobals.d_select_mode == sel_curvepoint
        || g_qeglobals.d_select_mode == sel_terrainpoint )
      && g_qeglobals.d_num_move_points > 0 )
    {
        MoveSelection( 0, 0, a );
        g_nUpdateBits = -1;
    }
    else
    {
        Select_Move( a, 1 );
        g_nUpdateBits = -1;
    }
}

// REGION SELECTION handlers — the IDB thunks (0x426340/0x426360/0x426370/0x426350)
// are 5-byte jmps straight to the cores (no undo wrapper: a selection change, not an
// edit). Each core takes the single selected brush as a box, deletes it, then
// (re)selects the brushes matching the box test (see select.cpp).
extern void Select_CompleteTall();   // select.cpp (0x490170)
extern void Select_PartialTall();    // select.cpp (0x4903D0)
extern void Select_Touching_R();     // select.cpp (0x490520)
extern void Select_Inside_R();       // select.cpp (0x490650)

void CMainFrame::OnSelectionCompleteTall()   // 0x426340 — Select Complete Tall (32984)
{
    Select_CompleteTall();
}

void CMainFrame::OnSelectionPartialTall()    // 0x426360 — Select Partial Tall (32983)
{
    Select_PartialTall();
}

void CMainFrame::OnSelectionTouching()       // 0x426370 — Select Touching (32986)
{
    Select_Touching_R();
}

void CMainFrame::OnSelectionInside()         // 0x426350 — Select Inside (33008)
{
    Select_Inside_R();
}

// ════════════════════════════════════════════════════════════════════════════
// MENU-WRAPPER BATCH - thin On* handlers forwarding to already-ported cores, each
// transcribed from the IDB (EAs below).
extern void    Select_Delete();                               // select.cpp (0x48E760)
extern void    Brush_AutoCaulk();                             // csg.cpp    (0x47E0F0)
extern void    Select_ChangeBrushToolflags( int set, int clear );// select.cpp (0x491890)
extern void    Undo_AddEntity_W( entity_s *def );            // undo.cpp   (0x45E990)
extern undo_s *g_lastundo;                                   // undo.cpp   (0x23F162C)

// CMainFrame::OnSelectionDelete (0x425690) - Edit->Delete (33003 / Backspace).  Undo-brackets
// the selection, then per selected brush adds its OWNER entity + that entity's whole brush-def
// list to undo (so a Delete that empties an entity can be restored), then Select_Delete().
// The per-iteration AddEntity + def-list walk IS the binary's inlined Undo_AddEntity_W.
void CMainFrame::OnSelectionDelete()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "delete" );
    Undo_AddBrushList( &selected_brushes );
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        if ( g_lastundo )
            Undo_AddEntity_W( (entity_s *)i->owner->def );
        else
            Sys_Printf( "Undo_AddEntity: no last undo.\n" );
    }
    Select_Delete();
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// CMainFrame::OnSelectionAutoCaulk (0x425600) — Selection→CSG→Auto Caulk (33220).
// Undo-bracketed Brush_AutoCaulk (caulks selected-brush faces hidden by neighbours).
void CMainFrame::OnSelectionAutoCaulk()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "Auto caulk" );
    Undo_AddBrushList( &selected_brushes );
    Brush_AutoCaulk();
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// CMainFrame::OnSelectionMakeWeaponclip (0x426240) — Make Weapon Clip (196). Sibling
// of Make Detail/Structural — sets the weapon-clip contents bits via Select_ChangeBrushType.
void CMainFrame::OnSelectionMakeWeaponclip()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "make weaponclip" );
    Undo_AddBrushList( &selected_brushes );
    Select_ChangeBrushType( 0x8002080, 0 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// CMainFrame::OnSelectionMakeNonColliding (0x426280) — Make Non-Colliding (197).
void CMainFrame::OnSelectionMakeNonColliding()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "make noncolliding" );
    Undo_AddBrushList( &selected_brushes );
    Select_ChangeBrushType( 134217732, 0 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// CMainFrame::OnSelectionMakeSplitCoplanar (0x4262C0) — Make Split Coplanar Geo (33223).
// Sets brush-face toolflag bit 256 (split-coplanar) via Select_ChangeBrushToolflags.
void CMainFrame::OnSelectionMakeSplitCoplanar()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "make split coplanar geo" );
    Undo_AddBrushList( &selected_brushes );
    Select_ChangeBrushToolflags( 256, 0 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// CMainFrame::OnSelectionMakeDontSplitCoplanar (0x426300) — Make Don't Split Coplanar (33224).
void CMainFrame::OnSelectionMakeDontSplitCoplanar()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "make don't split coplanar geo" );
    Undo_AddBrushList( &selected_brushes );
    Select_ChangeBrushToolflags( 0, 256 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// CMainFrame::OnViewCenter (0x423C50) — View→Center (32953 / End). Levels the camera
// (zero pitch/roll) and snaps its yaw to the nearest 22.5° increment, then repaints
// the camera + Z views. Pure inline camera math (no core dependency); transcribed
// verbatim — note floor((yaw+11)/22.5)*22.5 is the binary's snap (NOT a rounding helper).
void CMainFrame::OnViewCenter()
{
    if ( m_pCamWnd )
    {
        m_pCamWnd->camera.angles[0] = 0.0f;
        m_pCamWnd->camera.angles[2] = 0.0f;
        m_pCamWnd->camera.angles[1] =
            (float)( floor( ( m_pCamWnd->camera.angles[1] + 11.0 ) / 22.5 ) * 22.5 );
    }
    g_nUpdateBits |= ( W_CAMERA | W_XY_OVERLAY );   // binary: |= 5u (0x1 | 0x4)
}

extern LRESULT Texture_ShowInuse();   // texwnd.cpp (0x45B850) — mark/count in-use textures

// CMainFrame::OnViewUpfloor (0x424700) / OnViewDownfloor (0x423ED0) — View→Up/Down Floor
// (32954/32955). Snap the camera onto the nearest brush surface above/below it
// (CCamWnd::Cam_ChangeFloor, now ported in camwnd.cpp). Verbatim one-line handlers.
void CMainFrame::OnViewUpfloor()
{
    CCamWnd::Cam_ChangeFloor( m_pCamWnd, 1 );
}
void CMainFrame::OnViewDownfloor()
{
    CCamWnd::Cam_ChangeFloor( m_pCamWnd, 0 );
}

// CMainFrame::OnTexturesShowinuse (0x424B20) — Textures→Show In Use (32974). Mark only
// the textures the map references, then redraw the texture browser. Faithful to the
// binary: wait-cursor → Texture_ShowInuse() → RedrawWindow(m_pTexWnd) if present.
void CMainFrame::OnTexturesShowinuse()
{
    HCURSOR prev = SetCursor( LoadCursorA( 0, (LPCSTR)IDC_WAIT ) );
    Texture_ShowInuse();
    (void)prev;
    if ( m_pTexWnd )
        ::RedrawWindow( m_pTexWnd->m_hWnd, 0, 0,
                        RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
}

// CMainFrame::OnTexturesShowall (0x42B440) — Textures→Show All (32973 / Ctrl-A).  The command
// is OVERLOADED in the binary: with a trigger in the selection it is the Script-Group "add
// colour" command instead, and only otherwise does it un-hide every registered material.
extern LRESULT Texture_ShowAll();   // texwnd.cpp (0x45b730)
void CMainFrame::OnTexturesShowall()
{
    if ( ScriptGroup_SelectionHasTrigger() )
    {
        ScriptGroup_SyncGroupKeyToTeam();
        ScriptGroup_AddColorToSelection();
        return;
    }
    Texture_ShowAll();
    if ( m_pTexWnd )
        ::RedrawWindow( m_pTexWnd->m_hWnd, 0, 0,
                        RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
}

// ════════════════════════════════════════════════════════════════════════════
// TEXTURE REFRESH / RESOLUTION / WINDOW-SCALE cluster (Textures menu).  The image-reload
// backend (R_ReloadImages / R_UpdateMipMap) is in gfx_d3d/r_image.cpp under KISAK_RADIANT;
// kisak's imageGlobals.imageHashTable[32768] IS the binary's flat imageGlobals[32768].

void __cdecl R_ReloadImages();   // gfx_d3d/r_image.cpp (idb R_ReloadImages 0x513D70; decl in r_image.h)
void __cdecl R_UpdateMipMap();    // gfx_d3d/r_image.cpp (idb R_UpdateMipMap  0x5139A0; decl in r_image.h)

// CMainFrame::OnTextureRefresh (0x428B50) — Textures→Refresh Textures (33204, F5).  Reload
// every loose-file image from disk so edited TGA/DDS/IWI shows up without restarting, then
// invalidate all views.  Verbatim: R_ReloadImages(); g_nUpdateBits = -1.
void CMainFrame::OnTextureRefresh()
{
    R_ReloadImages();
    g_nUpdateBits = -1;
}

// CMainFrame::PicMip (0x420860) — apply g_qeglobals.d_savedinfo.d_picmip to the r_picmip*
// dvars and check the matching Texture-Resolution radio item.  Verbatim from the binary,
// adapted to kisak's Dvar_SetIntByName(name, value) arg order (the binary lists them
// swapped; see gfxwrapper.cpp).  d_picmip is 0..3 (Maximum..Low); the menu ids are
// 36115+d_picmip.  (Bump/spec are pinned to 3, exactly as the binary.)
void CMainFrame::PicMip()
{
    int picmip = g_qeglobals.d_savedinfo.d_picmip;
    iassert( picmip >= 0 && picmip <= 3 );   // idb: menuID in [36115, 36118]
    Dvar_SetIntByName( "r_picmip",      picmip );
    Dvar_SetIntByName( "r_picmip_spec", 3 );
    Dvar_SetIntByName( "r_picmip_bump", 3 );
    CMenu *menu = GetMenu();
    if ( menu )
    {
        menu->CheckMenuItem( 36115, MF_BYCOMMAND | MF_UNCHECKED );
        menu->CheckMenuItem( 36116, MF_BYCOMMAND | MF_UNCHECKED );
        menu->CheckMenuItem( 36117, MF_BYCOMMAND | MF_UNCHECKED );
        menu->CheckMenuItem( 36118, MF_BYCOMMAND | MF_UNCHECKED );
        menu->CheckMenuItem( 36115 + picmip, MF_BYCOMMAND | MF_CHECKED );
    }
}

// CMainFrame::OnTextureResolution (0x424340) — Textures→Texture Resolution (36115..36118 =
// Maximum/High/Normal/Low).  If the picked level differs from the current d_picmip, store it,
// push the r_picmip dvars (PicMip), propagate them into imageGlobals.picmip* (R_UpdateMipMap),
// reload every loose-file image at the new mip level (R_ReloadImages), and invalidate all
// views (g_nUpdateBits = -1).  Verbatim from the binary (id − 36115 == the picmip level).
void CMainFrame::OnTextureResolution( UINT nID )
{
    int level = (int)nID - 36115;
    if ( level != g_qeglobals.d_savedinfo.d_picmip )
    {
        g_qeglobals.d_savedinfo.d_picmip = level;
        PicMip();
        R_UpdateMipMap();
        R_ReloadImages();
        g_nUpdateBits = -1;
    }
}

// ── Textures→Texture Filter submenu (33226..33230) — 0x424200 / 0x424250 / 0x4242A0 /
//    Each handler: find the malleable "r_textureMode" dvar and set its string from source
//    (the binary's 2-arg Dvar_SetStringFromSource passes source 0 == DVAR_SOURCE_INTERNAL),
//    or register it fresh with flags 0x4000 (DVAR_EXTERNAL) + description "External Dvar";
//    then `or g_nUpdateBits, 1` (== W_CAMERA).
//    KISAK: CoD4's renderer consults r_textureMode in a global sampler-filter override;
//    kisak's CoD3 renderer bakes min/mag/mip into each technique's GfxStateBits.samplerState
//    and has no such override, so the dvar is written faithfully but nothing consumes it.
static void Radiant_SetTextureMode( const char *mode )
{
    const dvar_s *v = Dvar_FindVar( "r_textureMode" );          // idb Dvar_FindMalleableVar 0x4B0F00
    if ( v )
        Dvar_SetStringFromSource( (dvar_s *)v, (char *)mode, DVAR_SOURCE_INTERNAL );
    else
        Dvar_RegisterString( "r_textureMode", mode, 0x4000, "External Dvar" );
    g_nUpdateBits |= W_CAMERA;
}
void CMainFrame::OnTextureFilterNearest()     { Radiant_SetTextureMode( "nearest" ); }      // 33226 (0x424200)
void CMainFrame::OnTextureFilterLinear()      { Radiant_SetTextureMode( "linear" ); }       // 33227 (0x424250)
void CMainFrame::OnTextureFilterBilinear()    { Radiant_SetTextureMode( "bilinear" ); }     // 33228 (0x4242A0)
void CMainFrame::OnTextureFilterTrilinear()   { Radiant_SetTextureMode( "trilinear" ); }    // 33229 (0x4242F0)
void CMainFrame::OnTextureFilterAnisotropic() { Radiant_SetTextureMode( "anisotropic" ); }  // 33230 (0x424380)

// ── Textures→Render Method (32990..32994) — the binary's ON_COMMAND_RANGE handler
//    OnRendermethodCaseTextures (0x4243D0) is a one-liner: `Texture_SetMode(nID)`.
//    Texture_SetMode (0x45A520, texwnd.cpp) maps the five menu ids onto camera draw modes
//    0..4 (Wireframe / Fullbright / Normal-based fake lighting / View-based fake lighting /
//    Case textures) and stores the picked id in d_savedinfo.iTextMenu.
//    NOTE this is a DIFFERENT feature from OnRenderMethod{Material,Lightmap,Smoothing}
//    (33232/33233/36100 → Material_SetMode) — those stay exactly where they are.
extern void Texture_SetMode( int iTexMenu );                                // texwnd.cpp (0x45A520)
void CMainFrame::OnRendermethodCaseTextures( UINT nID ) { Texture_SetMode( (int)nID ); }

// CMainFrame::CheckTextureScale (0x42AF50) — check the picked Texture-Window-Scale radio item
// (32894..32898 = 200/100/50/25/10%), persist prefs, reset the browser scroll, and repaint the
// texture window.  Verbatim; the binary's raw `g_nUpdateBits |= 0x10` is W_TEXTURE (qedefs
// macros are aligned to the binary values, so the symbol equals the raw 0x10).
void CMainFrame::CheckTextureScale( UINT uIDCheckItem )
{
    CMenu *menu = GetMenu();
    if ( menu )
    {
        menu->CheckMenuItem( 32898, MF_BYCOMMAND | MF_UNCHECKED );   // 10%
        menu->CheckMenuItem( 32897, MF_BYCOMMAND | MF_UNCHECKED );   // 25%
        menu->CheckMenuItem( 32896, MF_BYCOMMAND | MF_UNCHECKED );   // 50%
        menu->CheckMenuItem( 32895, MF_BYCOMMAND | MF_UNCHECKED );   // 100%
        menu->CheckMenuItem( 32894, MF_BYCOMMAND | MF_UNCHECKED );   // 200%
        menu->CheckMenuItem( uIDCheckItem, MF_BYCOMMAND | MF_CHECKED );
    }
    Prefs_SavePrefs( g_PrefsDlg );   // idb CPrefsDlg::SavePrefs(g_PrefsDlg) 0x44f280
    Texture_ResetPosition();
    g_nUpdateBits |= W_TEXTURE;   // idb `or g_nUpdateBits, 10h` (W_TEXTURE == 0x10)
}

// The five Texture-Window-Scale menu items (32894..32898).  Each stores the percentage in
// g_PrefsDlg->m_nTextureWindowScale then CheckTextureScale(id).  Verbatim (0x42B020..0x42AFE0).
void CMainFrame::OnTexturesTexturewindowscale10()  { g_PrefsDlg->m_nTextureWindowScale = 10;  CheckTextureScale( 32898 ); }
void CMainFrame::OnTexturesTexturewindowscale25()  { g_PrefsDlg->m_nTextureWindowScale = 25;  CheckTextureScale( 32897 ); }
void CMainFrame::OnTexturesTexturewindowscale50()  { g_PrefsDlg->m_nTextureWindowScale = 50;  CheckTextureScale( 32896 ); }
void CMainFrame::OnTexturesTexturewindowscale100() { g_PrefsDlg->m_nTextureWindowScale = 100; CheckTextureScale( 32895 ); }
void CMainFrame::OnTexturesTexturewindowscale200() { g_PrefsDlg->m_nTextureWindowScale = 200; CheckTextureScale( 32894 ); }

// ════════════════════════════════════════════════════════════════════════════
// INSPECTOR TAB SWITCHING - each command toggles the right-column inspector to its mode (or
// hides the inspector if it is already in that mode + visible), then SetInspectorMode.

// Common toggle body (the shape shared by OnViewEntity 0x423f00 / OnViewTexture 0x424440
// / OnFilterDlg 0x42b7a0): in the integrated styles (1/2, !=floating/4-pane), if the
// inspector is already in <mode> + visible → hide it; otherwise show it + SetInspectorMode.
// In styles 0/3 the texture/console modes are separate windows (the no-op gate inside
// SetInspectorMode handles that); entity/filters still toggle.
static void Radiant_ToggleInspectorMode( int mode )
{
    HWND ent = g_qeglobals.d_hwndEntity;
    if ( !ent )
        return;
    if ( ::IsWindowVisible( ent ) && inspector_mode == mode )
    {
        ::ShowWindow( ent, SW_HIDE );
        // Return focus to the main frame so the NEXT hotkey press is dispatched (the frame now
        // has ON_WM_KEYDOWN).  Focusing the frame (not a D3D swap-chain view) is safe.  Without
        // this, hiding the focused popup could leave focus nowhere and the toggle worked only once.
        if ( g_pParentWnd && g_pParentWnd->GetSafeHwnd() )
            ::SetFocus( g_pParentWnd->GetSafeHwnd() );
        return;
    }
    ::ShowWindow( ent, SW_SHOWNORMAL );
    CEntityWnd_SetInspectorMode( mode );
}

// CMainFrame::OnViewEntity (0x423f00) — View→Toggle→Entity View (33017).
void CMainFrame::OnViewEntity()      { Radiant_ToggleInspectorMode( INSPECTOR_ENTITY ); }
// CMainFrame::OnViewTexture (0x424440) — Textures→Texture inspector tab (33018).
void CMainFrame::OnViewTextureMode() { Radiant_ToggleInspectorMode( INSPECTOR_TEXTURE ); }
// CMainFrame::OnViewConsole (0x423e10) — View→Toggle→Console View (33016, hotkey O).
void CMainFrame::OnViewConsole()     { Radiant_ToggleInspectorMode( INSPECTOR_CONSOLE ); }
// CMainFrame::OnFilterDlg (0x42b7a0) — View→Filter Settings (33104).
void CMainFrame::OnFilterDlg()       { Radiant_ToggleInspectorMode( INSPECTOR_FILTER ); }

// ── Textures-menu Usage / Locale / Surface-type filter command handlers ─────────
// The FillTextureMenu-built submenus (ids base+arrayIndex) dispatch here.  Each thin
// wrapper subtracts the submenu's id base to recover the filter index and forwards to the
// texwnd.cpp core.  IDB CMainFrame::OnFilterUsage 0x4243e0 / OnFilterLocale 0x424400 /
// OnFilterSurfaceType 0x424420 (each: `add nID, -base; call TexWnd_*Filter`).
extern void TexWnd_UsageFilter( int index );             // texwnd.cpp (0x45B3B0)
extern void TexWnd_localFilter( int index );             // texwnd.cpp (0x45B490)
extern void TexWnd_SurfaceTypeFilter( unsigned int index ); // texwnd.cpp (0x45B570)
void CMainFrame::OnFilterUsage( UINT nID )       { TexWnd_UsageFilter( (int)nID - 60000 ); }        // 0x4243ed
void CMainFrame::OnFilterLocale( UINT nID )      { TexWnd_localFilter( (int)nID - 60256 ); }        // 0x42440d
void CMainFrame::OnFilterSurfaceType( UINT nID ) { TexWnd_SurfaceTypeFilter( (unsigned int)( nID - 60512 ) ); } // 0x42442d

// ════════════════════════════════════════════════════════════════════════════
// SELECTION TRANSFORMS - Clone + Flip/Rotate X/Y/Z; each transcribed from the IDB.
extern void Clone_Selection( float gridSize );                              // select.cpp (0x48F0D0)
extern void DoFlip( int axis, const char *opName );                         // (0x424F30) — body below
extern void Select_GetMid( float *mid );                                    // select.cpp (0x48FC70)
extern void Select_RotateAxis( int axis, float deg, float (*rot_around)[4][3] ); // select.cpp (0x48FF40)
extern void Select_FlipAxis( int axis );                                    // select.cpp (0x48FD50)
extern void Select_ApplyMatrix_SelectedBrushes( int bSnap, float *mat, float deg, char bSwap ); // select.cpp (0x48FD10)
extern void sub_47B940( brush_t *def );                                     // brush.cpp (Brush_UpdateSpecialMaterialFlag, real)
extern int  UpdateSelection( int wParam, eclass_t *cls );                   // win_ent.cpp (0x497180)

// CMainFrame::OnSelectionClone (0x425480) — Selection→Clone (33001). Clones the
// selection (in-memory clone, see select.cpp note), then refreshes render flags on
// every selected + active brush DEF (sub_47B940 = Brush_UpdateSpecialMaterialFlag, now
// a real port in brush.cpp — refreshes the 2D back-face-cull hint from the face materials).
void CMainFrame::OnSelectionClone()
{
    Clone_Selection( grid_sizes[g_qeglobals.d_gridsize] );
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
        sub_47B940( i->def );
    for ( selbrush_t *j = active_brushes.next; j != &active_brushes; j = j->next )
        sub_47B940( j->def );
}

// CMainFrame::OnEditCopybrush (0x4286B0) — Edit→Copy (33039). Serialise the selection
// to the in-app clipboard via the active XY view.
void CMainFrame::OnEditCopybrush()
{
    if ( m_pActiveXY )
        m_pActiveXY->Copy();
}

// CMainFrame::OnEditPastebrush (0x4286D0) — Edit→Paste (33040). Re-parse the clipboard
// (placing + selecting), then refresh the special-material flag on every selected +
// active brush DEF (sub_47B940 = Brush_UpdateSpecialMaterialFlag, real port; identical
// tail to OnSelectionClone).
void CMainFrame::OnEditPastebrush()
{
    if ( m_pActiveXY )
        m_pActiveXY->Paste();
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
        sub_47B940( i->def );
    for ( selbrush_t *j = active_brushes.next; j != &active_brushes; j = j->next )
        sub_47B940( j->def );
}

// CMainFrame::OnBrushFlipx/y/z (0x4250A0/0x4250C0/0x4250E0) — Brush→Flip→X/Y/Z
// (32956/57/58). Each forwards to DoFlip (select.cpp), which undo-brackets the
// selection, mirrors it across the axis through the pivot (Select_FlipAxis), and for
// fixed-size entities also flips their `angles` key by 180°.
void CMainFrame::OnBrushFlipx() { DoFlip( 0, "flip X" ); }
void CMainFrame::OnBrushFlipy() { DoFlip( 1, "flip Y" ); }
void CMainFrame::OnBrushFlipz() { DoFlip( 2, "flip Z" ); }

// CMainFrame::OnBrushRotatex/y/z (0x425100/0x425190/0x425220) — Brush→Rotate→X/Y/Z
// (32959/60/61). The canonical transform pattern: undo bracket → Select_GetMid pivot →
// build the 90° rotation matrix (Select_RotateAxis) → apply to every selected brush
// (Select_ApplyMatrix_SelectedBrushes) → refresh selection → undo end.
static void Radiant_RotateSelection( int axis, const char *opName )
{
    if ( selected_brushes.next == &selected_brushes )
        return;
    Undo_ClearRedo();
    Undo_GeneralStart( opName );
    Undo_AddBrushList( &selected_brushes );
    float rot_around[4][3];
    Select_GetMid( rot_around[0] );
    Select_RotateAxis( axis, 90.0f, (float (*)[4][3])rot_around );
    Select_ApplyMatrix_SelectedBrushes( 0, rot_around[0], 90.0f, 0 );
    g_nUpdateBits = -1;
    UpdateSelection( -1, 0 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

void CMainFrame::OnBrushRotatex() { Radiant_RotateSelection( 0, "rotate X" ); }
void CMainFrame::OnBrushRotatey() { Radiant_RotateSelection( 1, "rotate Y" ); }
void CMainFrame::OnBrushRotatez() { Radiant_RotateSelection( 2, "rotate Z" ); }

// ═════════════════════════════════════════════════════════════════════════════════
// TOOLBAR COMMAND HANDLERS (IDR_TOOLBAR152).  MFC auto-disables toolbar buttons whose command
// id has no ON_COMMAND handler, so every button needs one.  Each toggles a pref/mode and
// reflects it on its button via TB_CHECKBUTTON (the binary's SendMessage(TB_CHECKBUTTON,
// cmdId, state)); bodies match the IDB 1:1.
extern void Brush_FlipTexture( int axis );          // select.cpp (0x492280)
extern void Brush_RotateTexture( int deg );         // select.cpp (0x4929F0)
extern void Material_SetMode( int iMode );          // texwnd.cpp (0x45B910)
extern void CMainFrame_UpdatePatchToolbarButtons(); // select.cpp (0x42AA70)
extern void Patch_InsDelToggle();                   // pmesh.cpp  (0x447E50)
extern int  g_qeglobals_redispersePatchVerts;       // engine_stubs.cpp (0x25D5A6B)
extern char g_nScaleHow;                            // drag.cpp   (0x23F16DC)
extern bool g_bScaleMode;                           // drag.cpp   (0x23F16DA)
extern bool g_bRotateMode;                          // drag.cpp   (0x23F16D9)

// TB_CHECKBUTTON helper — set the pressed/checked state of a toolbar button by command id.
static inline void TbCheck( CToolBar &bar, UINT cmdId, BOOL on )
{
    bar.SendMessage( TB_CHECKBUTTON, cmdId, on );
}

// ── Textures→Flip/Rotate (toolbar) ──────────────────────────────────────────────
void CMainFrame::OnTextureFlipX()    { Brush_FlipTexture( 0 ); }      // 0x42BF40 (33135)
void CMainFrame::OnTextureFlipY()    { Brush_FlipTexture( 1 ); }      // 0x42BF50 (33136)
void CMainFrame::OnTextureRotate90() { Brush_RotateTexture( 90 ); }   // 0x42BF60 (33182)

// ── Edit→Cycle Layer (0x424010, 33238) — advance the current material layer 0→1→2→0. ─
void CMainFrame::OnEditLayerCycle()
{
    Material_SetMode( ( g_qeglobals.current_edit_layer + 1 ) % 3 );
}

// ── Toggle Camera Movement Mode (0x429EB0, 32936) — cycle camera_mode 0→1→2→0. ─────
void CMainFrame::OnToggleCameraMovementMode()
{
    if ( ++g_PrefsDlg->camera_mode > 2 )
        g_PrefsDlg->camera_mode = 0;
    TbCheck( m_wndToolBar, 32936, g_PrefsDlg->camera_mode != 0 );
    Prefs_SavePrefs( g_PrefsDlg );
}

// ── View→Cubic Clipping (0x428F90, 32817) — toggle m_bCubicClipping + menu check. ──
void CMainFrame::OnViewCubicclipping()
{
    g_PrefsDlg->m_bCubicClipping ^= 1;
    if ( CMenu *menu = GetMenu() )
        menu->CheckMenuItem( 32817, g_PrefsDlg->m_bCubicClipping ? MF_CHECKED : MF_UNCHECKED );
    TbCheck( m_wndToolBar, 32817, g_PrefsDlg->m_bCubicClipping );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits |= 1u;                               // W_CAMERA
}

// ── View→Change (0x426400, 32781) — cycle the active XY view type XY→XZ→YZ→XY.
//    (SetViewType is a stub in this port, so the rotation is cosmetic until it lands;
//    the handler is faithful and PositionView/repaint run.) ───────────────────────────
void CMainFrame::OnViewChange()
{
    if ( m_nCurrentStyle == 2 || !m_pXYWnd )
        return;
    // m_nViewType is ED_VIEW (2=XY top, 1=XZ front, 0=YZ side).  Cycle XY→XZ→YZ→XY,
    // calling SetViewType with the matching EViewType (XY=0/XZ=1/YZ=2).
    switch ( m_pXYWnd->m_nViewType )
    {
    case 2:  m_pXYWnd->SetViewType( CXYWnd::XZ ); break;   // XY  → XZ
    case 1:  m_pXYWnd->SetViewType( CXYWnd::YZ ); break;   // XZ  → YZ
    default: m_pXYWnd->SetViewType( CXYWnd::XY ); break;   // YZ  → XY
    }
    m_pXYWnd->PositionView();
    g_nUpdateBits |= 2u;                               // W_XY
}

// ── Free mouse-rotation toggle (0x428570, 32810) ───────────────────────────────────
void CMainFrame::OnSelectMouserotate()
{
    if ( !m_pActiveXY )
        return;
    if ( g_bClipMode )
        OnViewClipper();
    if ( g_bRotateMode )
    {
        g_bRotateMode = false;
        m_pActiveXY->RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
        TbCheck( m_wndToolBar, 32810, FALSE );
        // If any non-patch brush is selected, rebuild brush display data.
        for ( selbrush_t *v3 = selected_brushes.next; v3 != &selected_brushes; v3 = v3->next )
        {
            if ( !v3->patch ) { Map_BuildBrushData(); break; }
        }
    }
    else if ( m_pActiveXY->SetRotateMode( 1 ) )
        TbCheck( m_wndToolBar, 32810, TRUE );
    else
        TbCheck( m_wndToolBar, 32810, FALSE );
}

// ── Mouse-scale mode toggle (0x428D20, 32813) ──────────────────────────────────────
void CMainFrame::OnSelectMousescale()
{
    if ( !m_pActiveXY )
        return;
    if ( g_bClipMode )
        OnViewClipper();
    if ( g_bRotateMode )
    {
        g_bRotateMode = false;
        m_pActiveXY->RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
        TbCheck( m_wndToolBar, 32813, FALSE );
    }
    if ( g_bScaleMode )
    {
        g_bScaleMode = false;
        m_pActiveXY->RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
        TbCheck( m_wndToolBar, 32813, FALSE );
    }
    else
    {
        g_bScaleMode = true;
        m_pActiveXY->RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
        TbCheck( m_wndToolBar, 32813, TRUE );
    }
}

// ── Per-axis scale-lock (0x428BF0 DoScaleLock; X/Y/Z = 32814/32815/32816) ───────────
// Bit values: X=1, Y=2, Z=4 in g_nScaleHow.  Verbatim from the IDB (register-arg order
// a1@eax, a3@edx, a2@ebx, this@esi mapped to a normal signature).
static void DoScaleLock( int a1, UINT a3, UINT a2, CMainFrame *self,
                         int a5, int a6, UINT wParam )
{
    CToolBar &bar = self->m_wndToolBar;
    if ( g_nScaleHow )
    {
        if ( g_nScaleHow == a5 )
        {
            g_nScaleHow = (char)( a6 | a1 );
            TbCheck( bar, a3, FALSE );
            TbCheck( bar, wParam, FALSE );
            TbCheck( bar, a2, TRUE );
        }
        else if ( ( g_nScaleHow & a1 ) != 0 && ( g_nScaleHow & a6 ) != 0 )
        {
            g_nScaleHow = 0;
            TbCheck( bar, a3, FALSE );
            TbCheck( bar, wParam, FALSE );
            TbCheck( bar, a2, FALSE );
        }
        else
        {
            int v8 = ( g_nScaleHow & a5 ) ? ( a5 ^ g_nScaleHow ) : ( a5 | g_nScaleHow );
            g_nScaleHow = (char)v8;
            TbCheck( bar, a2, ( a5 & v8 ) == 0 );
        }
    }
    else
    {
        g_nScaleHow = (char)( a6 | a1 );
        TbCheck( bar, a3, FALSE );
        TbCheck( bar, wParam, FALSE );
        TbCheck( bar, a2, TRUE );
    }
}

void CMainFrame::OnScalelockX() { DoScaleLock( 4, 32816, 32814, this, 1, 2, 32815 ); } // 0x428BC0
void CMainFrame::OnScalelockY() { DoScaleLock( 1, 32814, 32815, this, 2, 4, 32816 ); } // 0x428B60
void CMainFrame::OnScalelockZ() { DoScaleLock( 1, 32814, 32816, this, 4, 2, 32815 ); } // 0x428B90

// ── Don't-select-curves toggle (0x429920, 32852) ───────────────────────────────────
void CMainFrame::OnDontselectcurve()
{
    bool wasOn = ( g_PrefsDlg->m_bSelectCurves == 1 );
    g_PrefsDlg->m_bSelectCurves ^= 1;
    TbCheck( m_wndToolBar, 32852, wasOn );             // button shows "don't select" = !m_bSelectCurves
    Prefs_SavePrefs( g_PrefsDlg );
}

// ── Patch render/edit toggles ──────────────────────────────────────────────────────
void CMainFrame::OnPatchWireframe()                   // 0x42A300 (32857) — cycle 0→1→2→0
{
    if ( ++g_PrefsDlg->patch_wireframe > 2 )
        g_PrefsDlg->patch_wireframe = 0;
    TbCheck( m_wndToolBar, 32857, g_PrefsDlg->patch_wireframe != 0 );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnPatchWeld()                        // 0x42A400 (32858)
{
    bool wasOn = ( g_PrefsDlg->g_bPatchWeld != 0 );
    g_PrefsDlg->g_bPatchWeld ^= 1;
    TbCheck( m_wndToolBar, 32858, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnPatchDrilldown()                   // 0x42A510 (32865)
{
    bool wasOn = ( g_PrefsDlg->patch_drill_down != 0 );
    g_PrefsDlg->patch_drill_down ^= 1;
    TbCheck( m_wndToolBar, 32865, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

// ── Patch curve-edit vert-lock modes (0x42B4F0 / 0x42B510 / 0x42B530) ───────────────
void CMainFrame::ToggleLockPatchVertMode()            // 33140
{
    g_qeglobals.bLockPatchVerts   = !g_qeglobals.bLockPatchVerts;
    g_qeglobals.bUnlockPatchVerts = 0;
    CMainFrame_UpdatePatchToolbarButtons();
}

void CMainFrame::ToggleUnlockPatchVertMode()          // 33139
{
    g_qeglobals.bLockPatchVerts   = 0;
    g_qeglobals.bUnlockPatchVerts = !g_qeglobals.bUnlockPatchVerts;
    CMainFrame_UpdatePatchToolbarButtons();
}

void CMainFrame::OnCycleTerrainEdge()                 // 0x42B530 (33141)
{
    g_qeglobals.bLockPatchVerts   = 0;
    g_qeglobals.bUnlockPatchVerts = 0;
    if ( g_qeglobals.d_select_mode == sel_cycle_edge_direction_quad )
    {
        g_qeglobals.d_select_mode = sel_brush;
        CMainFrame_UpdatePatchToolbarButtons();
    }
    else
    {
        select_t prev = g_qeglobals.d_select_mode;
        g_qeglobals.d_select_mode = sel_cycle_edge_direction_quad;
        CMainFrame_UpdatePatchToolbarButtons();
        if ( prev == sel_addpoint )
            sub_43ECB0();                              // Patch_FinishCurveDrag
    }
    CMainFrame_UpdatePatchToolbarButtons();
}

// ── Filter / selection-visibility toggles (all the TB_CHECKBUTTON pref togglers) ────
void CMainFrame::OnToggleTextureAlphaRendering()      // 0x429F10 (33138)
{
    bool wasOn = ( g_PrefsDlg->camera_masked == 1 );
    g_PrefsDlg->camera_masked ^= 1;
    TbCheck( m_wndToolBar, 33138, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits |= 1u;
}

void CMainFrame::OnDisableSelectionOfEntities()       // 0x429F60 (33142)
{
    bool wasOn = ( g_PrefsDlg->entities_off == 1 );
    g_PrefsDlg->entities_off ^= 1;
    TbCheck( m_wndToolBar, 33142, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits |= 1u;
}

void CMainFrame::OnDisableSelectionOfSky()            // 0x429FB0 (33169)
{
    bool wasOn = ( g_PrefsDlg->sky_brush_off == 1 );
    g_PrefsDlg->sky_brush_off ^= 1;
    TbCheck( m_wndToolBar, 33169, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits |= 1u;
}

void CMainFrame::OnToggleDrawSurfs()                  // 0x42A040 (33144)
{
    bool wasOn = ( g_PrefsDlg->draw_toggle == 1 );
    g_PrefsDlg->draw_toggle ^= 1;
    TbCheck( m_wndToolBar, 33144, !wasOn );
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
        sub_47B940( i->def );
    for ( selbrush_t *j = active_brushes.next; j != &active_brushes; j = j->next )
        sub_47B940( j->def );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnSelectableModels()                 // 0x42A280 (33156)
{
    bool wasOn = ( g_PrefsDlg->m_bSelectableModels == 1 );
    g_PrefsDlg->m_bSelectableModels ^= 1;
    TbCheck( m_wndToolBar, 33156, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnPlantModel()                       // 0x42A0E0 (33159)
{
    bool wasOn = ( g_PrefsDlg->m_bDropModel == 1 );
    g_PrefsDlg->m_bDropModel ^= 1;
    TbCheck( m_wndToolBar, 33159, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnForceZeroDropHeight()              // 0x42A000 (33206)
{
    bool wasOn = ( g_PrefsDlg->m_bForceZeroDropHeight == 1 );
    g_PrefsDlg->m_bForceZeroDropHeight ^= 1;
    TbCheck( m_wndToolBar, 33206, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
}

void CMainFrame::OnOrientToFloor()                    // 0x4258F0 (33195)
{
    bool wasOn = ( g_PrefsDlg->m_bOrientModel == 1 );
    g_PrefsDlg->m_bOrientModel ^= 1;
    TbCheck( m_wndToolBar, 33195, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnTolerantWeld()                     // 0x42A130 (33155)
{
    bool wasOn = ( g_PrefsDlg->m_bTolerantWeld == 1 );
    g_PrefsDlg->m_bTolerantWeld ^= 1;
    TbCheck( m_wndToolBar, 33155, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnVertSnapModel()                    // 0x42A180 (33207)
{
    bool wasOn = ( g_PrefsDlg->m_bVertSnapModel == 1 );
    g_PrefsDlg->m_bVertSnapModel ^= 1;
    TbCheck( m_wndToolBar, 33207, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnVertSnapBrush()                    // 0x42A1D0 (33208)
{
    bool wasOn = ( g_PrefsDlg->m_bVertSnapBrush == 1 );
    g_PrefsDlg->m_bVertSnapBrush ^= 1;
    TbCheck( m_wndToolBar, 33208, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

void CMainFrame::OnVertSnapPrefab()                   // 0x42A220 (33209)
{
    bool wasOn = ( g_PrefsDlg->m_bVertSnapPrefab == 1 );
    g_PrefsDlg->m_bVertSnapPrefab ^= 1;
    TbCheck( m_wndToolBar, 33209, !wasOn );
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits = -1;
}

// ── Patch→Redisperse control points toggle (0x42A990, 32872) ───────────────────────
void CMainFrame::OnPatchRedisperse()
{
    Patch_InsDelToggle();
    TbCheck( m_wndToolBar, 32872, g_qeglobals_redispersePatchVerts != 0 );
    g_nUpdateBits = -1;
}

// ── Patch→Advanced Edit Dialog (0x42BC90, 33130) — toggle the terrain-paint dialog ──
extern void AdvPatchEdit_Toggle( CWnd *parent );   // patchdialog.cpp
void CMainFrame::OnAdvancedEditDlg()
{
    AdvPatchEdit_Toggle( this );
    g_nUpdateBits |= 1u;
}

// ── Misc→Cycle Preview Models (0x42BDD0, 35005) ────────────────────────────────────
// Advance w_cyclePreviewMode 0→1→2→3→4→0 (0 = off).  For each selected brush: clear any
// existing preview-model flag (brushFlags&0x100) and bump the entity-def version so its
// model re-instances; then, if the new mode is on AND the eclass has a model name for that
// mode slot (eclass->default_model_name[mode]), re-arm the preview flag.  Verbatim from the
// IDB (the dead `if(!v1)` re-test is omitted — v1 = old+1 is never 0 for the 0..4 range).
extern void Entity_RebuildBounds( entity_s *e );    // entity.cpp (0x485390)
void CMainFrame::OnMiscCyclePreviewModels()
{
    if ( (unsigned short)( ++g_qeglobals.w_cyclePreviewMode ) > 4u )
        g_qeglobals.w_cyclePreviewMode = 0;
    BOOL on = ( g_qeglobals.w_cyclePreviewMode != 0 );
    TbCheck( m_wndToolBar, 35005, on );
    if ( CMenu *menu = GetMenu() )
        menu->CheckMenuItem( 35005, on ? MF_CHECKED : MF_UNCHECKED );

    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        int brushFlags = i->brushFlags;
        if ( ( brushFlags & 0x100 ) != 0 )
        {
            entity_s *def = (entity_s *)i->owner->def;
            i->brushFlags = brushFlags & ~0x100;
            def->modelClass = nullptr;
            ++*(unsigned short *)&def->version;       // LOWORD(version)++ — force re-instance
            *(unsigned char *)&i->def->unk01 = 0;     // LOBYTE(unk01) = 0  (clear brush dirty)
            Entity_RebuildBounds( def );
        }
        if ( g_qeglobals.w_cyclePreviewMode )
        {
            entity_s *def = (entity_s *)i->owner->def;
            const char *modelName = *(const char **)( (char *)&def->eclass->default_model_name
                                      + (unsigned short)g_qeglobals.w_cyclePreviewMode * 4 );
            if ( modelName )
            {
                i->brushFlags |= 0x100;
                def->modelClass = nullptr;
                ++*(unsigned short *)&def->version;
                *(unsigned char *)&i->def->unk01 = 0;
            }
        }
    }
    g_nUpdateBits = -1;
}

// ── Drop Selected Relative-Z (0x425940, 35042) ─────────────────────────────────────
// In curve/terrain point-edit mode, drop the SELECTED control points to the floor while
// PRESERVING their relative Z offsets: find the lowest selected point, trace each point
// straight down, and shift it so the lowest sits on the surface and the rest keep their
// height above it.  Cancels mouse-rotate mode (m_pActiveXY, the IDB +0x814 member confirmed
// vs OnSelectMouserotate) and re-tessellates each selected patch.  One undo bracket.
extern void Patch_Rebuild( patchMesh_t *p, char doBounds );   // pmesh.cpp (0x438D80)
void CMainFrame::OnDropSelectedRelativeZ()
{
    if ( selected_brushes.next == &selected_brushes )
        return;

    Undo_ClearRedo();
    Undo_GeneralStart( "drop selection" );
    Undo_AddBrushList( &selected_brushes );
    Prefs_LoadPrefs( g_PrefsDlg );

    if ( ( g_qeglobals.d_select_mode == sel_curvepoint || g_qeglobals.d_select_mode == sel_terrainpoint )
         && g_qeglobals.d_num_move_points > 0 )
    {
        float lowestZ = g_qeglobals.d_move_points[0]->xyz[2];
        for ( int k = 0; k < g_qeglobals.d_num_move_points; ++k )
            if ( lowestZ > g_qeglobals.d_move_points[k]->xyz[2] )
                lowestZ = g_qeglobals.d_move_points[k]->xyz[2];

        float dir[3] = { 0.0f, 0.0f, -1.0f };
        for ( int k = 0; k < g_qeglobals.d_num_move_points; ++k )
        {
            drawVert_t *mp = g_qeglobals.d_move_points[k];
            float heightAboveLowest = mp->xyz[2] - lowestZ;   // keep this point's offset
            edTrace_t tr;
            Trace_AllDirectionsIfFailed( mp->xyz, &tr, dir, 4610 );
            if ( tr.hit.brush )
                g_qeglobals.d_move_points[k]->xyz[2] -= ( tr.dist - heightAboveLowest );
        }

        // Cancel mouse-rotate mode (same idiom as OnSelectMouserotate's rotate-off).
        if ( m_pActiveXY )
        {
            g_bRotateMode = false;
            m_pActiveXY->RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
            TbCheck( m_wndToolBar, 32810, FALSE );   // Free_rotation button off
        }

        for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
            Patch_Rebuild( i->patch->def, 1 );
    }

    g_nUpdateBits = -1;
    Undo_EndBrushList( &selected_brushes );   // the inlined per-brush undo-id stamp epilogue
    Undo_End();
}

// ── Entity-display-mode popup (OnShowEntities, toolbar 32915) ───────────────────────
// HandlePopup (0x41fb70): load MENU 160, pop up its submenu 0 at the cursor; the chosen
// item dispatches a WM_COMMAND to the frame (one of the 6 OnViewEntitiesas* handlers).
static void HandlePopup( CWnd *self, UINT menuId )
{
    POINT pt;
    GetCursorPos( &pt );
    CMenu menu;
    if ( menu.LoadMenu( menuId ) )
    {
        if ( CMenu *sub = menu.GetSubMenu( 0 ) )
            sub->TrackPopupMenu( TPM_RIGHTBUTTON, pt.x, pt.y, self );   // IDB flags=2
        menu.DestroyMenu();
    }
    self->SetFocus();
}

void CMainFrame::OnShowEntities() { HandlePopup( this, IDR_ENTITYPOPUP160 ); }   // 0x42B300

// 0x4241E0  CMainFrame::OnShowRegionsForSelected — build the per-pixel-light-preview
// CSG regions for every selected light (camwnd.cpp Regions_ForSelected), then flag a
// redraw.  The produced GfxLightRegionHull array (d_lightRegionHulls) is consumed by
// the parked per-pixel light draw (LightPreview_DrawLight, #26 layer C).
extern void Regions_ForSelected( CCamWnd *cam );   // camwnd.cpp 0x406F10
void CMainFrame::OnShowRegionsForSelected()
{
    Regions_ForSelected( m_pCamWnd );
    g_nUpdateBits |= 1u;
}

// 0x42BFD0  CMainFrame::OnSetAsActiveLayer (cmd 33955) — the Layers "Set as active layer"
// command.  The binary handler is a single `retn` (empty stub); ported verbatim as a no-op.
// Wired so 33955 no longer WRONGLY fires OnShowRegionsForSelected (that is now on 36125).
void CMainFrame::OnSetAsActiveLayer()
{
}

// SetEntityCheck (0x42B1F0) — re-check the main-menu item matching the active show-state.
// The exact m_nEntityShowState values were read from the IDB handler immediates (default
// 0x10010 = Skinned).  No-op for items absent from the port's menu bar.
void CMainFrame::SetEntityCheck()
{
    CMenu *menu = GetMenu();
    if ( !menu )
        return;
    int s = g_PrefsDlg->m_nEntityShowState;
    menu->CheckMenuItem( 32909, ( s == 0x1000 )  ? MF_CHECKED : MF_UNCHECKED );  // Bounding box
    menu->CheckMenuItem( 32916, ( s == 0x10001 ) ? MF_CHECKED : MF_UNCHECKED );  // Wireframe
    menu->CheckMenuItem( 32911, ( s == 0x101 )   ? MF_CHECKED : MF_UNCHECKED );  // Selected Wireframe
    menu->CheckMenuItem( 32912, ( s == 0x110 )   ? MF_CHECKED : MF_UNCHECKED );  // Selected Skinned
    menu->CheckMenuItem( 32913, ( s == 0x10010 ) ? MF_CHECKED : MF_UNCHECKED );  // Skinned
    menu->CheckMenuItem( 32914, ( s == 0x11010 ) ? MF_CHECKED : MF_UNCHECKED );  // Skinned and Boxed
}

// The 6 entity-display-mode setters (0x42B320..0x42B3E0): set the show-state bits, re-check
// the menu, persist, repaint.  Bit layout (from the IDB immediates): WIREFRAME=0x1,
// SKIN_MODEL=0x10, SELECTED_ONLY=0x100, BOXED=0x1000, SKINNED=0x10000.
void CMainFrame::OnViewEntitiesasBoundingbox()
{ g_PrefsDlg->m_nEntityShowState = 0x1000;  SetEntityCheck(); Prefs_SavePrefs( g_PrefsDlg ); g_nUpdateBits = -1; }
void CMainFrame::OnViewEntitiesasWireframe()
{ g_PrefsDlg->m_nEntityShowState = 0x10001; SetEntityCheck(); Prefs_SavePrefs( g_PrefsDlg ); g_nUpdateBits = -1; }
void CMainFrame::OnViewEntitiesasSelectedwireframe()
{ g_PrefsDlg->m_nEntityShowState = 0x101;   SetEntityCheck(); Prefs_SavePrefs( g_PrefsDlg ); g_nUpdateBits = -1; }
void CMainFrame::OnViewEntitiesasSelectedskinned()
{ g_PrefsDlg->m_nEntityShowState = 0x110;   SetEntityCheck(); Prefs_SavePrefs( g_PrefsDlg ); g_nUpdateBits = -1; }
void CMainFrame::OnViewEntitiesasSkinned()
{ g_PrefsDlg->m_nEntityShowState = 0x10010; SetEntityCheck(); Prefs_SavePrefs( g_PrefsDlg ); g_nUpdateBits = -1; }
void CMainFrame::OnViewEntitiesasSkinnedandboxed()
{ g_PrefsDlg->m_nEntityShowState = 0x11010; SetEntityCheck(); Prefs_SavePrefs( g_PrefsDlg ); g_nUpdateBits = -1; }

// ── Small modal utility dialogs (win_dlg.cpp) ─────────────────────────────────
// The binary's handlers DialogBoxParamA/DoModal a modal dialog; our ports are hand-built
// modeless popups (CMapInfo pattern; radiant.rc has no template).  The undo bracket the
// binary's OnSelectionArbitraryrotation (0x425300) wraps around DoModal lives inside the
// CArbRotateDlg OK handler (the popup is non-blocking, so it can't bracket here).
void CMainFrame::OnMiscFindbrush()             { CFindBrushDlg::Show(); }   // 0x424B96
void CMainFrame::OnMiscGoToPosition()          { CGoToDlg::Show(); }        // 0x424BB9
void CMainFrame::OnSelectionArbitraryrotation(){ CArbRotateDlg::Show(); }   // 0x425300

// ── Colors menu (DoColor is in win_dlg.cpp, IDB 0x499350) ─────────────────────
// Each thunk is DoColor(paletteIndex) then g_nUpdateBits = -1 (Sys_UpdateWindows).  DoColor
// already sets g_nUpdateBits = -1 on OK; the trailing assignment (kept verbatim from the
// binary) also forces a full repaint on cancel.  Palette indices decoded from each EA body.
void CMainFrame::OnTextureBackground()         { DoColor(0);  g_nUpdateBits = -1; }  // 0x424E40
void CMainFrame::OnColorsXyBackground()        { DoColor(1);  g_nUpdateBits = -1; }  // 0x424EC0
void CMainFrame::OnColorsMinor()               { DoColor(2);  g_nUpdateBits = -1; }  // 0x424EA0
void CMainFrame::OnColorsMajor()               { DoColor(3);  g_nUpdateBits = -1; }  // 0x424E80
void CMainFrame::OnColorsCameraBack()          { DoColor(4);  g_nUpdateBits = -1; }  // 0x424E60
void CMainFrame::OnColorsGridblock()           { DoColor(7);  g_nUpdateBits = -1; }  // 0x427320
void CMainFrame::OnColorsGridText()            { DoColor(8);  g_nUpdateBits = -1; }  // 0x4272C0
void CMainFrame::OnColorsBrush()               { DoColor(9);  g_nUpdateBits = -1; }  // 0x427240
void CMainFrame::OnColorsSelectedbrush()       { DoColor(10); g_nUpdateBits = -1; }  // 0x4272E0
void CMainFrame::OnColorsSelectedbrushCamera() { DoColor(11); g_nUpdateBits = -1; }  // 0x427300
void CMainFrame::OnColorsClipper()             { DoColor(12); g_nUpdateBits = -1; }  // 0x427280
void CMainFrame::OnColorsViewname()            { DoColor(13); g_nUpdateBits = -1; }  // 0x427340
void CMainFrame::OnColorsDetailBrush()         { DoColor(14); g_nUpdateBits = -1; }  // 0x427360
void CMainFrame::OnColorsToggleDrawSurfs()     { DoColor(15); g_nUpdateBits = -1; }  // 0x427380
void CMainFrame::OnColorsSelfaceCamera()       { DoColor(16); g_nUpdateBits = -1; }  // 0x4273E0
void CMainFrame::OnColorsFuncGroup()           { DoColor(17); g_nUpdateBits = -1; }  // 0x427400
void CMainFrame::OnColorsFuncCullGroup()       { DoColor(18); g_nUpdateBits = -1; }  // 0x427420
void CMainFrame::OnColorsWeaponclip()          { DoColor(19); g_nUpdateBits = -1; }  // 0x4273A0
void CMainFrame::OnColorsSizeInfo()            { DoColor(20); g_nUpdateBits = -1; }  // 0x427440
void CMainFrame::OnColorsModel()               { DoColor(21); g_nUpdateBits = -1; }  // 0x427460
void CMainFrame::OnColorsUnknown208()          { DoColor(22); g_nUpdateBits = -1; }  // 0x4273C0
void CMainFrame::OnColorsWireframe()           { DoColor(23); g_nUpdateBits = -1; }  // 0x4272A0
void CMainFrame::OnColorsFrozenLayers()        { DoColor(24); g_nUpdateBits = -1; }  // 0x427260

// ── Select Entity Color (33036 / K accel; IDB 0x424C10) ───────────────────────
// Seed a scratch palette slot from the edited entity's "_color" key (slot 6 if a LIGHT is
// selected, else slot 5), pop DoColor on that slot, and — while the entity inspector is
// shown — write the picked "r g b" back to the entity via the key/value fields + AddProp().
extern entity_s_def *edit_entity;                                    // win_ent.cpp (0x240A108)
extern int           inspector_mode;                                 // win_ent.cpp (0x240A110)
extern void          AddProp();                                      // win_ent.cpp (0x497490)
extern void          Win_SetEntityKeyValueFields( const char *key, const char *value ); // win_ent.cpp
void CMainFrame::OnMiscSelectentitycolor()
{
    if ( !edit_entity )
        return;

    // A LIGHT selected (eclass->classtype & 1) uses slot 6 (normalized), else slot 5.
    int slot = 5;
    for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
    {
        entity_s_def *def = (entity_s_def *)sb->owner->def;
        if ( ( def->eclass->classtype & 1 ) != 0 )
        {
            Sys_Printf( "Light selected, normalizing _color value.\n" );
            slot = 6;
            break;
        }
    }

    // Pre-seed colors[slot] from the entity's "_color" key (if present and 3-float).
    const char *colorVal = "";
    for ( epair_t *ep = edit_entity->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, "_color" ) )
        {
            colorVal = ep->value;
            break;
        }
    }
    float cr, cg, cb;
    if ( colorVal[0] && sscanf( colorVal, "%f %f %f", &cr, &cg, &cb ) == 3 )
    {
        g_qeglobals.d_savedinfo.colors[slot][0] = cr;
        g_qeglobals.d_savedinfo.colors[slot][1] = cg;
        g_qeglobals.d_savedinfo.colors[slot][2] = cb;
        g_qeglobals.d_savedinfo.colors[slot][3] = 1.0f;
    }

    if ( inspector_mode == INSPECTOR_ENTITY && DoColor( slot ) )
    {
        char rgb[108];
        sprintf( rgb, "%f %f %f",
                 g_qeglobals.d_savedinfo.colors[slot][0],
                 g_qeglobals.d_savedinfo.colors[slot][1],
                 g_qeglobals.d_savedinfo.colors[slot][2] );
        Win_SetEntityKeyValueFields( "_color", rgb );
        AddProp();
    }
    g_nUpdateBits = -1;
}

// ── Colors→Themes — whole-palette presets.  Every colors[i][c] transcribed verbatim from
// the binary (float constant-by-constant; the decompiler's shuffled store order is
// irrelevant — no slot is written twice within a theme).  Each ends g_nUpdateBits = -1. ──

// 0x427480 — "QE4 Original"
void CMainFrame::OnThemeQ4()
{
    vec4_t *c = g_qeglobals.d_savedinfo.colors;
    c[0][0]=0.25f;  c[0][1]=0.25f;  c[0][2]=0.25f;  c[0][3]=1.0f;
    c[1][0]=1.0f;   c[1][1]=1.0f;   c[1][2]=1.0f;   c[1][3]=1.0f;
    c[2][0]=0.75f;  c[2][1]=0.75f;  c[2][2]=0.75f;  c[2][3]=1.0f;
    c[3][0]=0.5f;   c[3][1]=0.5f;   c[3][2]=0.5f;   c[3][3]=1.0f;
    c[4][0]=0.25f;  c[4][1]=0.25f;  c[4][2]=0.25f;  c[4][3]=1.0f;
    c[7][0]=0.0f;   c[7][1]=0.0f;   c[7][2]=1.0f;   c[7][3]=1.0f;
    c[8][0]=0.0f;   c[8][1]=0.0f;   c[8][2]=0.0f;   c[8][3]=1.0f;
    c[9][0]=0.0f;   c[9][1]=0.0f;   c[9][2]=0.0f;   c[9][3]=1.0f;
    c[10][0]=1.0f;  c[10][1]=0.0f;  c[10][2]=0.0f;  c[10][3]=1.0f;
    c[11][0]=1.0f;  c[11][1]=0.25f; c[11][2]=0.25f; c[11][3]=0.25f;
    c[12][0]=0.0f;  c[12][1]=0.0f;  c[12][2]=1.0f;  c[12][3]=1.0f;
    c[13][0]=0.5f;  c[13][1]=0.0f;  c[13][2]=0.75f; c[13][3]=1.0f;
    c[14][0]=0.0f;  c[14][1]=0.60000002f; c[14][2]=0.0f; c[14][3]=1.0f;
    c[15][0]=0.1f;  c[15][1]=0.40000001f; c[15][2]=1.0f; c[15][3]=1.0f;
    c[16][0]=1.0f;  c[16][1]=0.25f; c[16][2]=0.25f; c[16][3]=0.25f;
    c[17][0]=0.75f; c[17][1]=0.75f; c[17][2]=0.75f; c[17][3]=1.0f;
    c[18][0]=0.75f; c[18][1]=0.75f; c[18][2]=0.75f; c[18][3]=1.0f;
    c[19][0]=0.5f;  c[19][1]=0.60000002f; c[19][2]=0.0f; c[19][3]=1.0f;
    c[20][0]=0.64999998f; c[20][1]=0.0f; c[20][2]=0.0f; c[20][3]=1.0f;
    c[21][0]=0.85000002f; c[21][1]=0.0f; c[21][2]=0.85000002f; c[21][3]=1.0f;
    c[22][0]=0.80000001f; c[22][1]=0.60000002f; c[22][2]=0.0f; c[22][3]=1.0f;
    c[23][0]=0.2f;  c[23][1]=0.17f; c[23][2]=0.14f; c[23][3]=1.0f;
    c[24][0]=0.75f; c[24][1]=0.0f;  c[24][2]=0.0f;  c[24][3]=1.0f;
    c[25][0]=0.5f;  c[25][1]=0.0f;  c[25][2]=0.0f;  c[25][3]=1.0f;
    c[26][0]=1.0f;  c[26][1]=1.0f;  c[26][2]=1.0f;  c[26][3]=1.0f;
    g_nUpdateBits = -1;
}

// 0x427760 — "Q3Radiant Original"
void CMainFrame::OnThemeQ3()
{
    vec4_t *c = g_qeglobals.d_savedinfo.colors;
    c[0][0]=0.25f;  c[0][1]=0.25f;  c[0][2]=0.25f;  c[0][3]=1.0f;
    c[1][0]=1.0f;   c[1][1]=1.0f;   c[1][2]=1.0f;   c[1][3]=1.0f;
    c[2][0]=1.0f;   c[2][1]=1.0f;   c[2][2]=1.0f;   c[2][3]=1.0f;
    c[3][0]=0.5f;   c[3][1]=0.5f;   c[3][2]=0.5f;   c[3][3]=1.0f;
    c[4][0]=0.25f;  c[4][1]=0.25f;  c[4][2]=0.25f;  c[4][3]=1.0f;
    c[7][0]=0.0f;   c[7][1]=0.0f;   c[7][2]=1.0f;   c[7][3]=1.0f;
    c[8][0]=0.0f;   c[8][1]=0.0f;   c[8][2]=0.0f;   c[8][3]=1.0f;
    c[9][0]=0.0f;   c[9][1]=0.0f;   c[9][2]=0.0f;   c[9][3]=1.0f;
    c[10][0]=1.0f;  c[10][1]=0.0f;  c[10][2]=0.0f;  c[10][3]=1.0f;
    c[11][0]=1.0f;  c[11][1]=0.25f; c[11][2]=0.25f; c[11][3]=0.25f;
    c[12][0]=0.0f;  c[12][1]=0.0f;  c[12][2]=1.0f;  c[12][3]=1.0f;
    c[13][0]=0.5f;  c[13][1]=0.0f;  c[13][2]=0.75f; c[13][3]=1.0f;
    c[14][0]=0.0f;  c[14][1]=0.60000002f; c[14][2]=0.0f; c[14][3]=1.0f;
    c[15][0]=0.40000001f; c[15][1]=0.60000002f; c[15][2]=1.0f; c[15][3]=1.0f;
    c[16][0]=1.0f;  c[16][1]=0.25f; c[16][2]=0.25f; c[16][3]=0.25f;
    c[17][0]=0.75f; c[17][1]=0.75f; c[17][2]=0.75f; c[17][3]=1.0f;
    c[18][0]=0.75f; c[18][1]=0.75f; c[18][2]=0.75f; c[18][3]=1.0f;
    c[19][0]=0.5f;  c[19][1]=0.60000002f; c[19][2]=0.0f; c[19][3]=1.0f;
    c[20][0]=0.64999998f; c[20][1]=0.0f; c[20][2]=0.0f; c[20][3]=1.0f;
    c[21][0]=0.85000002f; c[21][1]=0.0f; c[21][2]=0.85000002f; c[21][3]=1.0f;
    c[22][0]=0.80000001f; c[22][1]=0.60000002f; c[22][2]=0.0f; c[22][3]=1.0f;
    c[23][0]=0.2f;  c[23][1]=0.17f; c[23][2]=0.14f; c[23][3]=1.0f;
    c[24][0]=0.75f; c[24][1]=0.0f;  c[24][2]=0.0f;  c[24][3]=1.0f;
    c[25][0]=0.5f;  c[25][1]=0.0f;  c[25][2]=0.0f;  c[25][3]=1.0f;
    c[26][0]=1.0f;  c[26][1]=1.0f;  c[26][2]=1.0f;  c[26][3]=1.0f;
    g_nUpdateBits = -1;
}

// 0x427A40 — "Black and Green"
void CMainFrame::OnThemeBlackGreen()
{
    vec4_t *c = g_qeglobals.d_savedinfo.colors;
    c[0][0]=0.25f;  c[0][1]=0.25f;  c[0][2]=0.25f;  c[0][3]=1.0f;
    c[1][0]=0.0f;   c[1][1]=0.0f;   c[1][2]=0.0f;   c[1][3]=1.0f;
    c[2][0]=0.0f;   c[2][1]=0.0f;   c[2][2]=0.0f;   c[2][3]=1.0f;
    c[3][0]=0.30000001f; c[3][1]=0.5f; c[3][2]=0.5f; c[3][3]=1.0f;
    c[4][0]=0.25f;  c[4][1]=0.25f;  c[4][2]=0.25f;  c[4][3]=1.0f;
    c[7][0]=0.0f;   c[7][1]=0.0f;   c[7][2]=1.0f;   c[7][3]=1.0f;
    c[8][0]=1.0f;   c[8][1]=1.0f;   c[8][2]=1.0f;   c[8][3]=1.0f;
    c[9][0]=1.0f;   c[9][1]=1.0f;   c[9][2]=1.0f;   c[9][3]=1.0f;
    c[10][0]=1.0f;  c[10][1]=0.0f;  c[10][2]=0.0f;  c[10][3]=1.0f;
    c[11][0]=1.0f;  c[11][1]=0.25f; c[11][2]=0.25f; c[11][3]=0.25f;
    c[12][0]=0.0f;  c[12][1]=0.0f;  c[12][2]=1.0f;  c[12][3]=1.0f;
    c[13][0]=0.69999999f; c[13][1]=0.69999999f; c[13][2]=0.69999999f; c[13][3]=1.0f;
    c[14][0]=0.0f;  c[14][1]=0.60000002f; c[14][2]=0.0f; c[14][3]=1.0f;
    c[15][0]=0.40000001f; c[15][1]=0.60000002f; c[15][2]=1.0f; c[15][3]=1.0f;
    c[16][0]=1.0f;  c[16][1]=0.25f; c[16][2]=0.25f; c[16][3]=0.25f;
    c[17][0]=0.75f; c[17][1]=0.75f; c[17][2]=0.75f; c[17][3]=1.0f;
    c[18][0]=0.75f; c[18][1]=0.75f; c[18][2]=0.75f; c[18][3]=1.0f;
    c[19][0]=0.5f;  c[19][1]=0.60000002f; c[19][2]=0.0f; c[19][3]=1.0f;
    c[20][0]=0.64999998f; c[20][1]=0.0f; c[20][2]=0.0f; c[20][3]=1.0f;
    c[21][0]=0.85000002f; c[21][1]=0.0f; c[21][2]=0.85000002f; c[21][3]=1.0f;
    c[22][0]=0.80000001f; c[22][1]=0.60000002f; c[22][2]=0.0f; c[22][3]=1.0f;
    c[23][0]=0.2f;  c[23][1]=0.17f; c[23][2]=0.14f; c[23][3]=1.0f;
    c[24][0]=0.75f; c[24][1]=0.0f;  c[24][2]=0.0f;  c[24][3]=1.0f;
    c[25][0]=0.25f; c[25][1]=0.0f;  c[25][2]=0.0f;  c[25][3]=1.0f;
    c[26][0]=0.75f; c[26][1]=0.75f; c[26][2]=0.75f; c[26][3]=1.0f;
    g_nUpdateBits = -1;
}

// 0x427D30 — "Inverted"
void CMainFrame::OnThemeInverted()
{
    vec4_t *c = g_qeglobals.d_savedinfo.colors;
    c[0][0]=0.25f;  c[0][1]=0.25f;  c[0][2]=0.25f;  c[0][3]=1.0f;
    c[1][0]=0.0f;   c[1][1]=0.0f;   c[1][2]=0.0f;   c[1][3]=1.0f;
    c[2][0]=0.0f;   c[2][1]=0.0f;   c[2][2]=0.25f;  c[2][3]=1.0f;
    c[3][0]=0.0f;   c[3][1]=0.0f;   c[3][2]=0.5f;   c[3][3]=1.0f;
    c[4][0]=0.25f;  c[4][1]=0.25f;  c[4][2]=0.25f;  c[4][3]=1.0f;
    c[7][0]=0.0f;   c[7][1]=0.0f;   c[7][2]=1.0f;   c[7][3]=1.0f;
    c[8][0]=0.2f;   c[8][1]=0.69999999f; c[8][2]=0.55000001f; c[8][3]=1.0f;
    c[9][0]=0.5f;   c[9][1]=0.5f;   c[9][2]=0.5f;   c[9][3]=1.0f;
    c[10][0]=1.0f;  c[10][1]=0.0f;  c[10][2]=0.0f;  c[10][3]=1.0f;
    c[11][0]=1.0f;  c[11][1]=0.25f; c[11][2]=0.25f; c[11][3]=0.25f;
    c[12][0]=0.0f;  c[12][1]=0.0f;  c[12][2]=1.0f;  c[12][3]=1.0f;
    c[13][0]=0.69999999f; c[13][1]=0.69999999f; c[13][2]=0.0f; c[13][3]=1.0f;
    c[14][0]=0.0f;  c[14][1]=0.80000001f; c[14][2]=0.0f; c[14][3]=1.0f;
    c[15][0]=0.40000001f; c[15][1]=0.60000002f; c[15][2]=1.0f; c[15][3]=1.0f;
    c[16][0]=1.0f;  c[16][1]=0.25f; c[16][2]=0.25f; c[16][3]=0.25f;
    c[17][0]=0.75f; c[17][1]=0.75f; c[17][2]=0.75f; c[17][3]=1.0f;
    c[18][0]=0.75f; c[18][1]=0.75f; c[18][2]=0.75f; c[18][3]=1.0f;
    c[19][0]=0.5f;  c[19][1]=0.60000002f; c[19][2]=0.0f; c[19][3]=1.0f;
    c[20][0]=0.64999998f; c[20][1]=0.0f; c[20][2]=0.0f; c[20][3]=1.0f;
    c[21][0]=0.85000002f; c[21][1]=0.0f; c[21][2]=0.85000002f; c[21][3]=1.0f;
    c[22][0]=0.80000001f; c[22][1]=0.60000002f; c[22][2]=0.0f; c[22][3]=1.0f;
    c[23][0]=0.2f;  c[23][1]=0.17f; c[23][2]=0.14f; c[23][3]=1.0f;
    c[24][0]=0.75f; c[24][1]=0.0f;  c[24][2]=0.0f;  c[24][3]=1.0f;
    c[25][0]=0.25f; c[25][1]=0.0f;  c[25][2]=0.0f;  c[25][3]=1.0f;
    c[26][0]=0.2f;  c[26][1]=0.69999999f; c[26][2]=0.55000001f; c[26][3]=1.0f;
    g_nUpdateBits = -1;
}

// 0x428040 — "Gray"
void CMainFrame::OnThemeGrey()
{
    vec4_t *c = g_qeglobals.d_savedinfo.colors;
    c[0][0]=0.63f;  c[0][1]=0.63f;  c[0][2]=0.63f;  c[0][3]=1.0f;
    c[1][0]=0.63f;  c[1][1]=0.63f;  c[1][2]=0.63f;  c[1][3]=1.0f;
    c[2][0]=0.56999999f; c[2][1]=0.56999999f; c[2][2]=0.56999999f; c[2][3]=1.0f;
    c[3][0]=0.5f;   c[3][1]=0.5f;   c[3][2]=0.5f;   c[3][3]=1.0f;
    c[4][0]=0.63f;  c[4][1]=0.63f;  c[4][2]=0.63f;  c[4][3]=1.0f;
    c[7][0]=0.0f;   c[7][1]=0.0f;   c[7][2]=1.0f;   c[7][3]=1.0f;
    c[8][0]=0.0f;   c[8][1]=0.27000001f; c[8][2]=0.1f; c[8][3]=1.0f;
    c[9][0]=0.0f;   c[9][1]=0.02f;  c[9][2]=0.38f;  c[9][3]=1.0f;
    c[10][0]=0.25999999f; c[10][1]=1.0f; c[10][2]=0.63999999f; c[10][3]=1.0f;
    c[11][0]=0.25999999f; c[11][1]=1.0f; c[11][2]=0.63999999f; c[11][3]=0.25f;
    c[12][0]=0.0f;  c[12][1]=0.0f;  c[12][2]=1.0f;  c[12][3]=1.0f;
    c[13][0]=0.0f;  c[13][1]=0.27000001f; c[13][2]=0.1f; c[13][3]=1.0f;
    c[14][0]=0.66000003f; c[14][1]=0.67000002f; c[14][2]=1.0f; c[14][3]=1.0f;
    c[15][0]=0.1f;  c[15][1]=0.40000001f; c[15][2]=1.0f; c[15][3]=1.0f;
    c[16][0]=0.25999999f; c[16][1]=1.0f; c[16][2]=0.63999999f; c[16][3]=0.25f;
    c[17][0]=0.75f; c[17][1]=0.75f; c[17][2]=0.75f; c[17][3]=1.0f;
    c[18][0]=0.0f;  c[18][1]=0.5f;  c[18][2]=0.5f;  c[18][3]=1.0f;
    c[19][0]=0.79000002f; c[19][1]=0.80000001f; c[19][2]=0.1f; c[19][3]=1.0f;
    c[20][0]=0.0f;  c[20][1]=0.27000001f; c[20][2]=0.1f; c[20][3]=1.0f;
    c[21][0]=0.88f; c[21][1]=0.88999999f; c[21][2]=1.0f; c[21][3]=1.0f;
    c[22][0]=0.80000001f; c[22][1]=0.60000002f; c[22][2]=0.0f; c[22][3]=1.0f;
    c[23][0]=0.2f;  c[23][1]=0.17f; c[23][2]=0.14f; c[23][3]=1.0f;
    c[24][0]=0.75f; c[24][1]=0.0f;  c[24][2]=0.0f;  c[24][3]=1.0f;
    c[25][0]=0.0f;  c[25][1]=0.27000001f; c[25][2]=0.1f; c[25][3]=1.0f;
    c[26][0]=0.63f; c[26][1]=0.63f; c[26][2]=0.63f; c[26][3]=1.0f;
    g_nUpdateBits = -1;
}

// ── CMainFrame::OnDynamicLighting (0x429960) — "Dynamic Lighting" (menu 32854/0x8056) ─────────
// Verbatim: allocate + Cam_Init a fresh CCamWnd (the port's ctor IS CCamWnd::Cam_Init
// 0x402c40) and Create it as a FLOATING top-level popup - class "QCamera", empty title,
// WS_OVERLAPPEDWINDOW, 200x200 at (100,100), parent = the desktop, child id 12345.
// The binary's feature is DEAD: its CCamWnd::OnCreate hijacks g_qeglobals.d_hwndCamera, but
// the popup's hwnd is never registered with the renderer and all 5 R_MAX_WINDOWS slots are
// taken at startup, so the popup renders nothing.  KISAK: the port omits the d_hwndCamera
// hijack (that would regress the real camera); CCamWnd::OnPaint no-ops for the unregistered
// hwnd, matching the "renders nothing" outcome.  See RADIANT_MISSING_FUNCTIONS.md.
void CMainFrame::OnDynamicLighting()
{
    CCamWnd *cam = new CCamWnd();                 // operator new(0x33C) + Cam_Init (0x402c40)
    if ( !cam )
        return;
    CRect rect( 100, 100, 300, 300 );            // 0x4299ba — 200x200 at (100,100)
    CWnd *parent = CWnd::FromHandle( ::GetDesktopWindow() );   // 0x4299cb
    // CWnd::Create("QCamera", "", WS_OVERLAPPEDWINDOW, rect, desktop, 12345) — vtable Create@+0x54.
    cam->Create( "QCamera", "", WS_OVERLAPPEDWINDOW, rect, parent, 12345u, nullptr );   // 0x4299f9
    cam->ShowWindow( SW_SHOW );                   // 0x429a04
}

// Misc->Maya Export (33186 -> ExportToMaya 0x491b20).  The binary first pops the
// ExportToMayaProc options dialog (0x496080) then calls ExportToMaya with those flags.
// KISAK: radiant.rc has no IDD_MAYA template, so the options dialog is not built; the export
// runs with that dialog's DEFAULTS (Merge ON, UVs OFF, Quads ON, units=Inch -> scale 2.54,
// name "radiantImport.mel") into "<exeDir>\Maya\".  The emit pipeline is in mayaexport.cpp.
extern "C" void ExportToMaya( const char *dir, const char *outName,
                              char emitUVs, char groupAsBrush, char polyList, float scale );
void CMainFrame::OnMiscMayaExport()                                          // 0x423xxx → 0x491b20
{
    // Output dir = the editor exe's folder (the binary uses g_strAppPath; same intent).
    char exePath[MAX_PATH];
    GetModuleFileNameA( nullptr, exePath, sizeof( exePath ) );
    char *slash = strrchr( exePath, '\\' );
    if ( slash ) *slash = 0;                       // strip the exe name → directory
    char mayaDir[MAX_PATH];
    _snprintf( mayaDir, sizeof( mayaDir ), "%s\\Maya", exePath );
    CreateDirectoryA( mayaDir, nullptr );           // ensure "<exeDir>\Maya\" exists

    ExportToMaya( exePath, "radiantImport.mel",
                  /*emitUVs*/0, /*groupAsBrush*/1, /*polyList*/1, /*scale=inch*/2.54f );
}

// ── Help→About + File→Error file + Textures→Render Method ─────────────────────
extern void Pointfile_Errorfile_Public();      // errorfile.cpp (0x4100B0 wrapper)
extern void Pointfile_Clear();                 // points.cpp (0x410600)
extern void Pointfile_ResetPoints();           // points.cpp (s_num_points = 0)
extern int  s_errLogCount; // points.cpp (== s_errLogCount @0x1814CE8)
extern void Material_SetMode( int iMode );     // texwnd.cpp (0x45B910)
extern void Texture_SetMode( int iTexMenu );   // texwnd.cpp (0x45A520)

void CMainFrame::OnHelpAbout() { CAboutDlg::Show(); }   // 0x4264F0

// 0x423B40 — toggle the error-log display: if errors are loaded, clear them; else load
// the current map's .errlog (Pointfile_Errorfile parses + sorts + navigates to the first).
// The leading s_num_points = 0 clears any displayed leak path first (the binary does this).
void CMainFrame::OnErrorFile()
{
    Pointfile_ResetPoints();
    if ( s_errLogCount )
        Pointfile_Clear();
    else
        Pointfile_Errorfile_Public();
}

// ── Pointfile (leak trace) menu handlers ──────────────────────────────────────
extern FILE *Pointfile_Check();        // points.cpp (0x48ACB0) — load currentmap's .lin
extern int   Pointfile_GetNumPoints(); // points.cpp (s_num_points)
extern void  Pointfile_Next();         // points.cpp (0x48AA60)
extern void  Pointfile_Prev();         // points.cpp (0x48AB90)
extern void  Errorfile_NextError();    // errorfile.cpp (0x410670)
extern void  Errorfile_PrevError();    // errorfile.cpp (0x410710)

// 0x423B20 — File→Pointfile toggle: clear the error log, then toggle the .lin leak path
// (loaded → hide; hidden → load currentmap's .lin via Pointfile_Check).
void CMainFrame::OnPointfileOpen()
{
    Pointfile_Clear();
    if ( Pointfile_GetNumPoints() )
        Pointfile_ResetPoints();
    else
        Pointfile_Check();
}

// 0x424BC0 — Misc→Next leak spot: step forward through the pointfile if one is loaded,
// otherwise through the error log.
void CMainFrame::OnMiscNextleakspot()
{
    if ( Pointfile_GetNumPoints() )
        Pointfile_Next();
    else if ( s_errLogCount )
        Errorfile_NextError();
}

// 0x424BE0 — Misc→Previous leak spot: symmetric to OnMiscNextleakspot.
void CMainFrame::OnMiscPreviousleakspot()
{
    if ( Pointfile_GetNumPoints() )
        Pointfile_Prev();
    else if ( s_errLogCount )
        Errorfile_PrevError();
}

// Textures→Render Method radio group → Material_SetMode(0/1/2).
void CMainFrame::OnRenderMethodMaterial()  { Material_SetMode( 0 ); }
void CMainFrame::OnRenderMethodLightmap()  { Material_SetMode( 1 ); }
void CMainFrame::OnRenderMethodSmoothing() { Material_SetMode( 2 ); }

// ── LAYERED MATERIALS — the authoring tool palette (layeredmaterialwnd.cpp) ──────
// OnToggleLayeredMaterials (0x42BFE0) show/hides the frame (body identical to
// LayeredMaterialWnd_ToggleVisibility 0x4176B0); OnSaveLayeredMaterials (0x42C020) flushes the
// library to disk (CRC-gated).  The real window is NOT auto-created at startup, so
// lyrMtlWndGlob.hwnd may be NULL - ShowWindow(NULL,...) is a harmless no-op.
extern "C" int LayeredMaterialWnd_UntoggleLiveAdd();  // layeredmaterialwnd.cpp (sub_417440)
extern char LayeredMaterials_Save();                  // layeredmaterials.cpp (0x416F40)

void CMainFrame::OnToggleLayeredMaterials()
{
    if ( !::IsWindowVisible( lyrMtlWndGlob.hwnd ) )
    {
        ::ShowWindow( lyrMtlWndGlob.hwnd, SW_SHOW );
        return;
    }
    if ( (BYTE)lyrMtlWndGlob.liveAddActive )
        LayeredMaterialWnd_UntoggleLiveAdd();   // binary calls sub_417440 (un-toggle Live)
    ::ShowWindow( lyrMtlWndGlob.hwnd, SW_HIDE );
}

void CMainFrame::OnSaveLayeredMaterials()
{
    LayeredMaterials_Save();
}

// ── Brush_Print (0x47BB60) — debug-dump a brush's face planepts to the console ──────
//   Faithful transcription incl. the binary's quirk: the loop counter (the "Face %i" label)
//   steps by 2 while the face index steps by 1 and the loop tests label < faceCount — so it
//   prints the first ceil(faceCount/2) faces labelled 0,2,4,...  (a harmless debug quirk).
//   The binary's face stride is 464B (CoD4 face_t); the port uses sizeof(face_t) (232B).
static void Brush_Print( brush_t *def )
{
    for ( int i = 0; 2 * i < def->faceCount; ++i )
    {
        const float *pp = &def->faces[i].planepts[0][0];
        Sys_Printf( "Face %i\n", 2 * i );
        Sys_Printf( "%g %g %g\n", pp[0], pp[1], pp[2] );
        Sys_Printf( "%g %g %g\n", pp[3], pp[4], pp[5] );
        Sys_Printf( "%g %g %g\n", pp[6], pp[7], pp[8] );
    }
}

// CMainFrame::OnSelectionPrint (0x429110, cmd 33087) — Brush_Print every selected brush's def.
void CMainFrame::OnSelectionPrint()
{
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
        Brush_Print( i->def );
}

// ── WXY_Print (0x463AE0) — print the XY viewport to the printer ─────────────────────
//   PrintDlg (PD_RETURNDC) → StartDoc/StartPage → StretchBlt the XY window's client into the
//   printer's physical page → EndPage/EndDoc, with a MessageBox on each failure step.  Verbatim
//   from the disasm (Win32 GDI printing).
static void WXY_Print()
{
    PRINTDLGA pd;
    memset( &pd, 0, sizeof( pd ) );
    pd.lStructSize = sizeof( PRINTDLGA );   // binary uses 66 (= sizeof on VC7.1)
    pd.hwndOwner   = g_qeglobals.d_hwndXY;
    pd.Flags       = PD_RETURNDC;
    pd.hInstance   = 0;
    if ( !PrintDlgA( &pd ) || !pd.hDC )
    {
        MessageBoxA( GetActiveWindow(), "Print Error!\n\nCould not PrintDlg()", "Radiant", MB_OK | MB_ICONERROR );
        return;
    }

    DOCINFOA di;
    memset( &di, 0, sizeof( di ) );
    di.cbSize      = sizeof( DOCINFOA );    // binary uses 20
    di.lpszDocName = "QE4";
    if ( StartDocA( pd.hDC, &di ) <= 0 )
    {
        MessageBoxA( GetActiveWindow(), "Print Error!\n\nCould not StartDoc()", "Radiant", MB_OK | MB_ICONERROR );
        return;
    }
    if ( StartPage( pd.hDC ) <= 0 )
    {
        MessageBoxA( GetActiveWindow(), "Print Error!\n\nCould not StartPage()", "Radiant", MB_OK | MB_ICONERROR );
        return;
    }

    RECT rc;
    GetWindowRect( g_qeglobals.d_hwndXY, &rc );
    int srcW = rc.right - rc.left;
    int srcH = rc.bottom - rc.top;
    int dstW = GetDeviceCaps( pd.hDC, PHYSICALWIDTH )  - GetDeviceCaps( pd.hDC, PHYSICALOFFSETX );
    int dstH = GetDeviceCaps( pd.hDC, PHYSICALHEIGHT ) - GetDeviceCaps( pd.hDC, PHYSICALOFFSETY );
    HDC src = GetDC( g_qeglobals.d_hwndXY );
    StretchBlt( pd.hDC, 0, 0, dstW, dstH, src, 0, 0, srcW, srcH, SRCCOPY );
    ReleaseDC( g_qeglobals.d_hwndXY, src );

    if ( EndPage( pd.hDC ) <= 0 )
    {
        MessageBoxA( GetActiveWindow(), "Print Error!\n\nCould not EndPage()", "Radiant", MB_OK | MB_ICONERROR );
        return;
    }
    if ( EndDoc( pd.hDC ) <= 0 )
        MessageBoxA( GetActiveWindow(), "Print Error!\n\nCould not EndDoc()", "Radiant", MB_OK | MB_ICONERROR );
}

// CMainFrame::OnMiscPrintxy (0x424C00, cmd 33037) — thunk to WXY_Print.
void CMainFrame::OnMiscPrintxy()
{
    WXY_Print();
}

// ── PATCH menu — Patch_BrushToMesh primitives + Patch_AdjustSelected grid edits ──
// Each handler is the binary's Undo-bracketed wrapper (OnCurvePatch* 0x42A360.. /
// OnCurveInsert*/Delete* 0x42A560..); the cores live in pmesh.cpp (data layer).
extern void Patch_BrushToMesh( char bCone, unsigned char bBevel,
                               unsigned char bEndcap, char bSquare );   // pmesh.cpp (0x43ACC0)
extern void Patch_AdjustSelected( char bInsert, char bColumn, char bFlag ); // pmesh.cpp (0x444550)
extern void Patch_ToggleInverted();                                         // pmesh.cpp (0x4465C0)
extern void Patch_Transpose();                                              // pmesh.cpp (0x4491D0)
extern void Patch_NaturalizeSelected( bool unk, bool cap, float x, float y );// pmesh.cpp (0x447FD0)
extern void Select_SetTexture( float *out );                                // select.cpp (0x456D70)
extern void Select_Invert();                                                // select.cpp (0x493F10)
extern void Patch_DisperseColumns();                                        // pmesh.cpp (0x4443A0)
extern void Patch_DisperseRows();                                           // pmesh.cpp (0x444200)
extern void Patch_InvertTexture( char axis );                               // pmesh.cpp (0x446680)

static void Radiant_PatchBrushToMesh( char cone, unsigned char bevel,
                                      unsigned char endcap, char square, const char *op )
{
    Undo_ClearRedo();
    Undo_GeneralStart( op );
    Undo_AddBrushList( &selected_brushes );
    Patch_BrushToMesh( cone, bevel, endcap, square );
    g_nUpdateBits = -1;
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}
static void Radiant_PatchAdjust( char ins, char col, char flag, const char *op )
{
    Undo_ClearRedo();
    Undo_GeneralStart( op );
    Undo_AddBrushList( &selected_brushes );
    Patch_AdjustSelected( ins, col, flag );
    g_nUpdateBits = -1;
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// Patch→Primitives→Dense Cylinder (32883, 0x42AB90) / Very Dense Cylinder (32884,
// 0x42AC40).  Both are the plain cylinder followed by NESTED, individually-bracketed
// Patch_AdjustSelected row edits — reproduced verbatim, including the nesting (the outer
// "dense cylinder" record stays open across the inner "add/insert (2) rows" records) and
// the repeated g_nUpdateBits = -1 writes.  Dense = 1 add + 1 insert; Very Dense = 2 of each.
static void Radiant_PatchDenseRows( int pairs, const char *op )
{
    Undo_ClearRedo();
    Undo_GeneralStart( op );
    Undo_AddBrushList( &selected_brushes );
    Patch_BrushToMesh( 0, 0, 0, 0 );
    for ( int i = 0; i < pairs; ++i )
    {
        Radiant_PatchAdjust( 1, 0, 1, "add (2) rows" );      // 0x42ABC6 / 0x42AC76
        Radiant_PatchAdjust( 1, 0, 0, "insert (2) rows" );   // 0x42ABF6 / 0x42ACA6
    }
    g_nUpdateBits = -1;
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}
void CMainFrame::OnCurvePatchdensetube()     { Radiant_PatchDenseRows( 1, "dense cylinder" ); }
void CMainFrame::OnCurvePatchverydensetube() { Radiant_PatchDenseRows( 2, "very dense cylinder" ); }

// Patch→Cycle Cap Texture (32905 / Shift+Ctrl+N, 0x42B1A0) — same shape as Naturalize
// but with the binary's (unk=1, cap=1) flags, i.e. it cycles the CAP texture projection.
void CMainFrame::OnCurveCyclecap()
{
    float x[2];
    Select_SetTexture( x );
    Patch_NaturalizeSelected( 1, 1, x[0], x[1] );
    g_nUpdateBits = -1;
}

// Patch→Insert→Add Terrain Row / Column (33153 / Shift+Ctrl+A, 0x42B080) — if a PATCH is
// selected, insert at the selected vertex pair; otherwise fall through to the entity-chain
// arm (clone the source entity onto the pair's midpoint and re-link).  Verbatim.
extern void Patch_InsertRemoveFromVertPair();     // pmesh.cpp (0x44BB90)
extern void Select_InsertMidpointEntity();        // select.cpp (0x48EB30)
void CMainFrame::OnAddTerrainRowColumn()
{
    if ( selected_brushes.next == &selected_brushes || !selected_brushes.next->patch )
        Select_InsertMidpointEntity();
    else
        Patch_InsertRemoveFromVertPair();
}

void CMainFrame::OnCurvePatchtube()   { Radiant_PatchBrushToMesh( 0, 0, 0, 0, "make curve cylinder" ); }
void CMainFrame::OnCurvePatchcone()   { Radiant_PatchBrushToMesh( 1, 0, 0, 0, "make curve cone" ); }
void CMainFrame::OnCurvePatchendcap() { Radiant_PatchBrushToMesh( 0, 0, 1, 0, "make end cap" ); }
void CMainFrame::OnCurvePatchbevel()  { Radiant_PatchBrushToMesh( 0, 1, 0, 0, "make bevel" ); }
void CMainFrame::OnCurvePatchsquare() { Radiant_PatchBrushToMesh( 0, 0, 0, 1, "square cylinder" ); }
void CMainFrame::OnCurveSquareBevel() { Radiant_PatchBrushToMesh( 0, 1, 0, 1, "square bevel" ); }
void CMainFrame::OnCurveSquareEndcap(){ Radiant_PatchBrushToMesh( 0, 0, 1, 1, "square endcap" ); }

// Curve->Insert menu.  "Insert" FRONT-inserts (bFlag=0 = subdivide the first segment), "Add"
// BACK-inserts (bFlag=1 = subdivide the last) - they are NOT identical.  From the binary's
// message map: 32874 Insert (2) Columns -> Patch_AdjustSelected(1,1,0); 32873 Add (2) Columns
// -> (1,1,1); rows mirror at 32876/32875.  The binary's SAME-NAMED OnCurveInsertcolumn (nID
// 32868, op "insert colum", flag 1) is a DIFFERENT handler - do not match this one to it.
void CMainFrame::OnCurveInsertcolumn()    { Radiant_PatchAdjust( 1, 1, 0, "insert (2) columns" ); } // 32874 front
void CMainFrame::OnCurveInsertAddcolumn() { Radiant_PatchAdjust( 1, 1, 1, "add (2) columns" ); }    // 32873 back
void CMainFrame::OnCurveInsertrow()       { Radiant_PatchAdjust( 1, 0, 0, "insert (2) rows" ); }    // 32876 front
void CMainFrame::OnCurveInsertAddrow()    { Radiant_PatchAdjust( 1, 0, 1, "add (2) rows" ); }       // 32875 back
void CMainFrame::OnCurveDeleteFirstcolumn(){ Radiant_PatchAdjust( 0, 1, 1, "delete first (2) columns" ); }
void CMainFrame::OnCurveDeleteLastcolumn() { Radiant_PatchAdjust( 0, 1, 0, "delete last (2) columns" ); }
void CMainFrame::OnCurveDeleteFirstrow()   { Radiant_PatchAdjust( 0, 0, 1, "delete first (2) rows" ); }
void CMainFrame::OnCurveDeleteLastrow()    { Radiant_PatchAdjust( 0, 0, 0, "delete last (2) rows" ); }
// Patch→Negative (32881): faithful thunk to Patch_ToggleInverted (IDB OnCurveNegative
// 0x42A7E0 is a pure thunk — NO Undo bracket, matching the binary).
void CMainFrame::OnCurveNegative()         { Patch_ToggleInverted(); }
// Patch→Matrix→Transpose (32906): IDB OnCurveMatrixTranspose 0x42B1E0 = Patch_Transpose()
// then g_nUpdateBits=-1 (no Undo bracket).
void CMainFrame::OnCurveMatrixTranspose()  { Patch_Transpose(); g_nUpdateBits = -1; }
// Patch→Naturalize (32890): IDB OnPatchNaturalize 0x42AE10 — fetch the default tex-repeat
// scale (Select_SetTexture), Patch_NaturalizeSelected(unk=0,cap=0,...) lays linear S/T,
// then g_nUpdateBits=-1. (Patch_NaturalizeSelected does its own Undo bracket.)
void CMainFrame::OnPatchNaturalize()       { float x[2]; Select_SetTexture( x ); Patch_NaturalizeSelected( 0, 0, x[0], x[1] ); g_nUpdateBits = -1; }
// Selection→Invert (33101 / Ctrl+I): IDB OnSelectionInvert 0x42B6F0 = Select_Invert()
// then g_nUpdateBits |= 0xB (swap active/selected lists + repaint XY/Z/camera).
void CMainFrame::OnSelectionInvert()       { Select_Invert(); g_nUpdateBits |= 0xB; }
// Patch→Redisperse Cols/Rows (32889/32888): IDB OnCurveRedisperse* 0x42AD80/0x42AD90 =
// Patch_Disperse{Columns,Rows}() then g_nUpdateBits = -1 (evenly redistribute control points).
void CMainFrame::OnCurveRedisperseCols()   { Patch_DisperseColumns(); g_nUpdateBits = -1; }
void CMainFrame::OnCurveRedisperseRows()   { Patch_DisperseRows();    g_nUpdateBits = -1; }
// Patch→Negative Texture X/Y (32899/32903): IDB OnCurveNegativeTexture{X,Y} 0x42A7F0/0x42A800
// = Patch_InvertTexture(0/1) (the worker sets g_nUpdateBits itself; pure thunks, no Undo bracket).
void CMainFrame::OnCurveNegativeTextureX() { Patch_InvertTexture( 0 ); }
void CMainFrame::OnCurveNegativeTextureY() { Patch_InvertTexture( 1 ); }

// ═════════════════════════════════════════════════════════════════════════════
// PATCH/CURVE OPERATION CLUSTER + TERRAIN row/col handlers; cores in pmesh.cpp, ids from the
// CMainFrame message map.
extern void        Patch_CapCurrent();                                   // pmesh.cpp 0x43AA20
extern void        Patch_Thicken( int amount, char bseam );              // pmesh.cpp 0x448700
extern void        SplitPatch();                                         // pmesh.cpp 0x44CC60
extern void        ExtrudeTerrainRow();                                  // pmesh.cpp 0x44BD40
extern void        RemoveTerrainRowCol();                                // pmesh.cpp 0x44BE10
extern brush_t    *PMESH_58( face_t *face, selbrush_t *ownerInst );      // pmesh.cpp 0x44D1F0
extern selbrush_t *PMESH_07_Width( selbrush_t *a1 );                     // pmesh.cpp 0x43B950
extern void        Patch_NaturalizeSelected( bool unk, bool cap, float x, float y );
extern void        Select_SetTexture( float *out );
extern selbrush_t  selected_brushes;
extern void        Brush_RemoveFromList( selbrush_t *b );                // brush.cpp
extern void        Brush_AddToList2( selbrush_t *b );                    // brush.cpp

// Curve→Cap (32885, IDB OnCurveCap 0x42AD40): auto-cap the single selected patch.
void CMainFrame::OnCurveCap()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "curve cap" );
    Undo_AddBrushList( &selected_brushes );
    Patch_CapCurrent();
    g_nUpdateBits = -1;
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// Patch→Cap naturalize (35040, IDB OnPatchCap 0x42AE50): lay cap texturing on the selection.
void CMainFrame::OnPatchCap()
{
    float x[2];
    Select_SetTexture( x );
    Patch_NaturalizeSelected( 1, 0, x[0], x[1] );
    g_nUpdateBits = -1;
}

// ── CDialogThick (IDD 0xA4) — Curve→Thicken parameters (thickness + seam) ─────
//   Hand-built like CPatchDensityDlg.  DDX (0x40C060): checkbox 1288 = "create seam"
//   (member @0x74, default 1), edit 1290 = thickness (member @0x78, default 8).
class CDialogThick : public CDialog
{
public:
    CDialogThick( CWnd *parent = nullptr ) : CDialog( IDD_THICKEN_PATCH, parent ), m_seam( 1 ), m_amount( 8 ) {}
    int m_seam;      // this+0x74 (DDX_Check 1288)
    int m_amount;    // this+0x78 (DDX_Text  1290)
protected:
    virtual void DoDataExchange( CDataExchange *pDX )   // 0x40C060
    {
        CDialog::DoDataExchange( pDX );
        DDX_Check( pDX, 1288, m_seam );
        DDX_Text ( pDX, 1290, m_amount );
    }
};

// Curve→Thicken (32904, IDB OnCurveThicken 0x42B0D0): pop CDialogThick, thicken on OK.
void CMainFrame::OnCurveThicken()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "curve thicken" );
    Undo_AddBrushList( &selected_brushes );
    {
        CDialogThick dlg;
        dlg.m_seam   = 1;
        dlg.m_amount = 8;
        if ( dlg.DoModal() == IDOK )
        {
            Patch_Thicken( dlg.m_amount, dlg.m_seam != 0 );
            g_nUpdateBits = -1;
        }
    }
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// ── CScaleDialog (IDD_SCALE 145) — Selection→Scale... parameters (X/Y/Z factors) ──
//   The binary's CScaleDialog overrides only the dtor, GetMessageMap (an EMPTY map) and
//   DoDataExchange (0x451110): DDX_Text 1089 -> +0x78 (X), 1142 -> +0x7C (Y), 1147 -> +0x74
//   (Z).  All three default to 1.0 (OnSelectScale's `fld1 / fst / fst / fstp` at 0x428411).
class CScaleDialog : public CDialog
{
public:
    CScaleDialog( CWnd *parent = nullptr )
        : CDialog( IDD_SCALE, parent ), m_z( 1.0f ), m_x( 1.0f ), m_y( 1.0f ) {}
    float m_z;   // this+0x74 (DDX_Text 1147)
    float m_x;   // this+0x78 (DDX_Text 1089)
    float m_y;   // this+0x7C (DDX_Text 1142)
protected:
    virtual void DoDataExchange( CDataExchange *pDX )   // 0x451110
    {
        CDialog::DoDataExchange( pDX );
        DDX_Text( pDX, 1147, m_z );
        DDX_Text( pDX, 1089, m_x );
        DDX_Text( pDX, 1142, m_y );
    }
};

// Selection->Scale... (32809, IDB OnSelectScale 0x4283D0) - pop the Scale dialog and, if the
// factors pass the two validations, Select_Scale the selection inside an undo bracket.
// PRECEDENCE, verbatim from 0x42844D..0x428481: the negative test is X < 0 || (Y < 0 && Z < 0),
// NOT a three-way OR (`fcom X` + `jnp` -> error, then `fcom Y` + `jp` SKIPS the Z test).  An
// original quirk: a lone negative Y or Z slips through to Select_Scale.
extern void Select_Scale( float sx, float sy, float sz );   // select.cpp (0x48FDC0)
extern int  OnlyPatchesSelected();                          // engine_stubs.cpp (0x447860)
void CMainFrame::OnSelectScale()
{
    CScaleDialog dlg;
    dlg.m_z = 1.0f;
    dlg.m_x = 1.0f;
    dlg.m_y = 1.0f;
    if ( dlg.DoModal() != IDOK )
        return;

    if ( dlg.m_x < 0.0f || ( dlg.m_y < 0.0f && dlg.m_z < 0.0f ) )
    {
        Sys_Printf( "Cannot scale by a negative value." );
        return;
    }
    if ( ( dlg.m_x == 0.0f || dlg.m_y == 0.0f || dlg.m_z == 0.0f ) && !OnlyPatchesSelected() )
    {
        Sys_Printf( "Can only scale patches by zero." );
        return;
    }

    Undo_ClearRedo();
    Undo_GeneralStart( "scale" );
    Undo_AddBrushList( &selected_brushes );
    Select_Scale( dlg.m_x, dlg.m_y, dlg.m_z );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
    g_nUpdateBits = -1;
}

// Selection→Clipper→Split selected (32794 / Shift+Enter, IDB OnSplitSelected 0x4271D0) —
// commit the clip keeping BOTH halves, inside an undo bracket.  Gated on an active XY view
// exactly as the binary is.
extern void Ed_SplitClip();                                 // xywnd.cpp (CXYWnd::SplitClip 0x46DD30)
void CMainFrame::OnSplitSelected()
{
    if ( m_pActiveXY )
    {
        Undo_ClearRedo();
        Undo_GeneralStart( "split selected" );
        Undo_AddBrushList( &selected_brushes );
        Ed_SplitClip();
        Undo_EndBrushList( &selected_brushes );
        Undo_End();
    }
}

// Physics→Cylinder (36113, IDB OnMakePhysCylinder 0x4291D0) / Physics→Box (36120,
// IDB OnMakePhysBox 0x429200) — undo-bracketed wrappers over the brush.cpp cores.
extern void Brush_MakePhysCylinder();   // brush.cpp (0x47C310)
extern void Brush_MakePhysBox();        // brush.cpp (0x47C180)
void CMainFrame::OnMakePhysCylinder()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "make physics cylinder" );
    Undo_AddBrushList( &selected_brushes );
    Brush_MakePhysCylinder();
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}
void CMainFrame::OnMakePhysBox()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "make physics box" );
    Undo_AddBrushList( &selected_brushes );
    Brush_MakePhysBox();
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// ══════════════════════════════════════════════════════════════════════════════
// The remaining CMainFrame singles, each ported from its own IDB body; the ones the binary
// leaves EMPTY are reproduced as empty (with the address), never as a silent TODO.
extern void Delete_Exportables();                // entity.cpp 0x487C30
extern void Select_HideUnselected2_unused();     // select.cpp 0x48EF40
extern void Patch_BendToggle();                  // pmesh.cpp  0x4478E0
extern void Patch_BendHandleTAB();               // pmesh.cpp  0x447980
extern void Patch_InsDelHandleTAB();             // pmesh.cpp  0x447F40
extern void Patch_SnapVertToGrid();              // pmesh.cpp  0x449280
extern void Patch_RedistributeVerts();           // pmesh.cpp  0x449930
extern void DoRedistPatchPts();                  // pmesh.cpp  0x449A90
extern void DoTurnTerrainEdges();                // pmesh.cpp  0x44B260
extern void SelectTargettedEntity();             // select.cpp 0x491440
extern int  g_bPatchBendMode;                    // pmesh.cpp  0x25D5B04
extern int  g_qeglobals_redispersePatchVerts;    // engine_stubs.cpp 0x25D5A6B
extern char PMESH_10( char bAdd, int dCol, int dRow );            // pmesh.cpp  0x43CB80
extern void Select_Brush( selbrush_t *b, char some_overwrite,
                          char bStatus, char center_grid_on_selection ); // select.cpp 0x48DCC0
extern void Select_Deselect( char bResetMode );                   // select.cpp 0x48E800

// 206 — Misc→Delete exportables (0x424E30), a pure thunk.
void CMainFrame::OnDeleteExportables() { Delete_Exportables(); }

// ── CCommandsDlg (IDD_DLG_COMMANDLIST 132) — Help→Command list... (32790) ──────
//   CCommandsDlg overrides DoDataExchange (0x40B850: DDX_Control 1036) and OnInitDialog
//   (0x40B890).  OnInitDialog sets ONE tab stop at 96 dialog units, opens "c:/commandlist.txt"
//   (CFile modeCreate|modeWrite, no exception object - a silent no-op when C:\ is not writable)
//   and appends "<name> \t<modifiers><key>" for every command to both the listbox and the file.
class CCommandsDlg : public CDialog
{
public:
    CCommandsDlg( CWnd *parent = nullptr ) : CDialog( IDD_DLG_COMMANDLIST, parent ) {}
protected:
    CListBox m_list;                                     // this+0x74 (DDX_Control 1036)
    virtual void DoDataExchange( CDataExchange *pDX )    // 0x40B850
    {
        CDialog::DoDataExchange( pDX );
        DDX_Control( pDX, 1036, m_list );
    }
    virtual BOOL OnInitDialog()                          // 0x40B890
    {
        CDialog::OnInitDialog();

        int tab = 96;
        m_list.SendMessage( LB_SETTABSTOPS, 1, (LPARAM)&tab );   // idb 0x40B8CB

        CFile fileout;
        const BOOL haveFile = fileout.Open( "c:/commandlist.txt",
                                            CFile::modeCreate | CFile::modeWrite, nullptr );

        Radiant_SeedCommandTable();
        for ( const RadiantCommand &c : g_radiantCommands )
        {
            // key name: the g_Keys entry, else the raw character (sub_40BBC0's %c).
            char keybuf[8];
            const char *keyName = nullptr;
            for ( const RadiantKeyName &k : g_radiantKeys )
                if ( k.vk == c.vk ) { keyName = k.name; break; }
            if ( !keyName )
            {
                keybuf[0] = (char)c.vk;
                keybuf[1] = '\0';
                keyName = keybuf;
            }

            char mods[64];
            mods[0] = '\0';
            if ( c.mods & 1 ) strcat( mods, "Shift" );
            if ( c.mods & 2 ) strcat( mods, mods[0] ? " + Alt"      : "Alt" );
            if ( c.mods & 4 ) strcat( mods, mods[0] ? " + Control"  : "Control" );
            if ( c.mods & 8 ) strcat( mods, mods[0] ? " + Left Win" : "Left Win" );
            if ( mods[0] )    strcat( mods, " + " );

            char line[320];
            _snprintf( line, sizeof( line ) - 1, "%s \t%s%s", c.name, mods, keyName );
            line[sizeof( line ) - 1] = '\0';
            m_list.AddString( line );

            if ( haveFile )
            {
                char fline[320];
                _snprintf( fline, sizeof( fline ) - 1, "%s \t\t\t%s%s", c.name, mods, keyName );
                fline[sizeof( fline ) - 1] = '\0';
                fileout.Write( fline, (UINT)strlen( fline ) );
                fileout.Write( "\r\n", 2 );
            }
        }
        if ( haveFile )
            fileout.Close();
        return TRUE;
    }
};

// 32790 — Help→Command list... (0x426E00): pop the modal CCommandsDlg.
void CMainFrame::OnHelpCommandlist()
{
    CCommandsDlg dlg( this );
    dlg.DoModal();
}

// 1085 — LinkSelectionToggle (Shift+O, 0x423EE0): flip the pref + persist it.
void CMainFrame::OnLinkKeepSelection()
{
    g_PrefsDlg->linking_keeps_selection = ( g_PrefsDlg->linking_keeps_selection == 0 );
    Prefs_SavePrefs( g_PrefsDlg );      // idb CPrefsDlg::SavePrefs 0x44F280
}

// 32776 — ToggleCamera (0x423A50): flip the camera-update preview flag.  (Distinct from
// 33069 OnTogglecamera, which show/hides the camera VIEW.)
void CMainFrame::ToggleCamera() { m_bCamPreview = !m_bCamPreview; }

// 32779 / 32780 — the two remaining HandlePopup targets (0x4263E0 / 0x4263F0).
void CMainFrame::OnPopupRenderMethod() { HandlePopup( this, IDR_POPUP_RENDER_METH ); }
void CMainFrame::OnPopupSelection()    { HandlePopup( this, IDR_POPUP_SELECTION ); }

// 32782 — View→Camera update (0x426450).  Its ON_UPDATE_COMMAND_UI was already wired;
// the command itself was not, so the check mark drew but clicking did nothing.
void CMainFrame::OnViewCameraupdate() { g_nUpdateBits |= W_CAMERA; }

// 32863 / 32864 — Patch→Primitives→Inverted End Cap / Inverted Bevel.  GENUINELY EMPTY
// in the binary (0x42A500 / 0x42A4F0 are single `retn`s) — reproduced as empty.
void CMainFrame::OnCurvePatchinvertedendcap() {}
void CMainFrame::OnCurvePatchinvertedbevel()  {}

// 32867/32868/32869/32870 — the SINGLE-row/column patch grid edits (Ctrl+Num+/- and the
// Shift variants).  Note these are NOT the Patch menu's "(2) rows/columns" commands
// (32873..32876) — different ids, different undo strings, and all four pass bFlag = 1.
void CMainFrame::OnCurveInsertrowSingle()    { Radiant_PatchAdjust( 1, 0, 1, "insert row" ); }     // 0x42A5B0
void CMainFrame::OnCurveInsertcolumnSingle() { Radiant_PatchAdjust( 1, 1, 1, "insert colum" ); }   // 0x42A560 (idb typo kept)
void CMainFrame::OnCurveDeleterowSingle()    { Radiant_PatchAdjust( 0, 0, 1, "delete row" ); }     // 0x42A650
void CMainFrame::OnCurveDeletecolumnSingle() { Radiant_PatchAdjust( 0, 1, 1, "delete column" ); }  // 0x42A600

// 32882 — Patch→Bend toggle (0x42A950): flip bend mode and re-check the toolbar button.
void CMainFrame::OnPatchBend()
{
    Patch_BendToggle();
    m_wndToolBar.SendMessage( TB_CHECKBUTTON, 32882 /*idb Patch_Bend_mode*/,
                              g_bPatchBendMode != 0 );
    g_nUpdateBits = -1;
}

// 32925 — HideByClassname (Shift+Alt+Ctrl+H, 0x42B6B0), a thunk.
void CMainFrame::OnHideUnselected2() { Select_HideUnselected2_unused(); }

// 32978 — Misc→Benchmark (0x424B70).  EMPTY in the binary (the camera-spin benchmark is
// compiled out); reproduced as empty so the menu item stops being silently unroutable.
void CMainFrame::OnMiscBenchmark() {}

// 33089 — Patch TAB (0x42A9E0).  In bend mode / redisperse mode the TAB steps that mode's
// state machine; otherwise it cycles to the NEXT brush of the selected brush's entity
// (worldspawn excluded).
void CMainFrame::OnPatchTab()
{
    if ( g_bPatchBendMode )
    {
        Patch_BendHandleTAB();
        return;
    }
    if ( g_qeglobals_redispersePatchVerts )
    {
        Patch_InsDelHandleTAB();
        return;
    }

    selbrush_t *cur = selected_brushes.next;
    if ( cur == &selected_brushes )
        return;
    entity_s *owner = cur->owner;
    if ( !_stricmp( ( (entity_s *)owner->def )->eclass->name, "worldspawn" ) )
        return;

    Select_Deselect( 1 );
    selbrush_t *head = &owner->brushes;
    selbrush_t *scan = owner->brushes.ownerNext;
    if ( scan != head )
    {
        // Walk to the node AFTER cur (the binary's do/while: stop once the previous
        // node was cur, or once the list wraps).
        bool wasCur;
        do
        {
            wasCur = ( cur == scan );
            scan = scan->ownerNext;
        } while ( !wasCur && scan != head );
    }
    if ( scan == head )
        scan = scan->ownerNext;      // skip the sentinel

    Select_Brush( scan, 0, 1, 0 );
    g_nUpdateBits = -1;
}

// 33090 — Patch ENTER (0x42A9D0).  EMPTY in the binary.
void CMainFrame::OnPatchEnter() {}

// 33091 — SelectSnapPointsToGrid (Ctrl+G, 0x42AE90).  With NOTHING selected this pops the
// "go to position" dialog; with a selection it snaps the patch control points to the grid
// inside an undo bracket.  (The odd pairing is the original's.)
void CMainFrame::OnGotoPos_unk()
{
    if ( selected_brushes.next == &selected_brushes )
    {
        CGoToDlg::Show();            // idb DialogBoxParamA(IDD_DIALOG_GOTO_POS, GoToDlgProc)
        return;
    }
    Undo_ClearRedo();
    Undo_GeneralStart( "snap to grid" );
    Undo_AddBrushList( &selected_brushes );
    Patch_SnapVertToGrid();
    g_nUpdateBits = -1;
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// 33165..33168 — the four "vertex select" nudges (Ctrl+Up/Down + two unbound siblings).
// Each is a bare PMESH_10(1, dCol, dRow) — bAdd = 1 (EXTEND the vertex selection), unlike
// the already-wired OnSelectionDragVertices arrows which pass bAdd = 0.
void CMainFrame::OnVertexSelectUp()    { PMESH_10( 1, 0, -1 ); }   // 0x426880
void CMainFrame::OnVertexSelectDown()  { PMESH_10( 1, 0,  1 ); }   // 0x4268A0
void CMainFrame::OnVertexSelectRight() { PMESH_10( 1, 1,  0 ); }   // 0x4268C0
void CMainFrame::OnVertexSelectLeft()  { PMESH_10( 1, -1, 0 ); }   // 0x4268E0

// 33170 — RedisperseVertices (Shift+F, 0x42A270), a thunk.
void CMainFrame::OnRedistPatchPoints() { DoRedistPatchPts(); }

// 33179 — AutoEdgeTurn (Alt+F2, 0x4294E0), a thunk.  (NOT the same command as the
// "Turn Terrain Edges" MENU item, which the original binds to 33142.)
void CMainFrame::OnTurnTerrainEdges() { DoTurnTerrainEdges(); }

// 33213 — DropVertices (Shift+Alt+Ctrl+D, 0x42AE00).
void CMainFrame::OnDropPatchVertices() { Patch_RedistributeVerts(); g_nUpdateBits = -1; }

// 36110 — SelectTargettedEntities (Ctrl+E, 0x425560), a thunk.
void CMainFrame::OnSelectTargettedEntity() { SelectTargettedEntity(); }

// 57602 / 57607 / 57609 — File→Close / Print / Print Preview.  All three are EMPTY in the
// binary (0x423A70 / 0x423B60 / 0x423B70) — Radiant does not implement them.
void CMainFrame::OnFileClose() {}
void CMainFrame::OnFilePrint() {}
void CMainFrame::OnFilePrintPreview() {}

// Curve→Split (33158, IDB OnSplitPatch 0x42B0C0): thunk.
void CMainFrame::OnSplitPatch() { SplitPatch(); }

// Terrain→Extrude Row/Col (33192, IDB 0x42B0A0): thunk.
void CMainFrame::ExtrudeTerrainRow2() { ExtrudeTerrainRow(); }

// Terrain→Remove Row/Col (33154, IDB 0x42B0B0): thunk.
void CMainFrame::OnRemoveTerrainRowColumn() { RemoveTerrainRowCol(); }

// Curve→Terrain (35041, IDB OnCurveToTerrain 0x429B30): convert selected patches to terrain.
void CMainFrame::OnCurveToTerrain()
{
    int n = 0;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( b->patch )
        {
            if ( !n )
            {
                Undo_ClearRedo();
                Undo_GeneralStart( "convert curve to terrain" );
                Undo_AddBrushList( &selected_brushes );
            }
            ++n;
            PMESH_07_Width( b );
        }
    }
    if ( !n )
    {
        Sys_Printf( "Curve to terrain: failed; no patches found in the selection.\n" );
        return;
    }
    Select_Delete();
    g_nUpdateBits = -1;
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
    Sys_Printf( va( "Curve to terrain: converted %i %s.\n", n, ( n <= 1 ) ? "curve" : "curves" ) );
}

// Selection→Connect (33021, IDB OnSelectionConnect 0x425510): in curve-point mode connect
//   patch vertices (ConnectVertices — patch-verts, still deferred); otherwise WeldMesh (now
//   ported), and if no weld happened, ConnectEntities_R.  Now wire-able because WeldMesh landed.
extern char WeldMesh();                                   // pmesh.cpp 0x44C8C0
extern void ConnectEntities_R();                          // select.cpp 0x48C530
extern void ConnectVertices();                            // pmesh.cpp 0x44A920
void CMainFrame::OnSelectionConnect()
{
    if ( g_qeglobals.d_select_mode == sel_curvepoint )
    {
        // ConnectVertices (0x44A920) — patch control-vertex connect/weld (now ported).
        ConnectVertices();
        return;
    }
    if ( !WeldMesh() )
    {
        ConnectEntities_R();
        UpdateSelection( -1, 0 );
    }
}

// Face→Terrain (36102, IDB OnFaceToTerrain 0x429BE0): convert selected faces to terrain patches.
void CMainFrame::OnFaceToTerrain()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "convert faces to terrain" );
    Undo_AddBrushList( &selected_brushes );

    int faceCount = g_SelectedFaces.GetSize();
    // collect the new terrain brushes (the binary uses a CArray<BrushInst*>; a plain
    // bounded buffer is faithful — one new brush per selected face).
    selbrush_t *newBrushes[256];
    int newCount = 0;
    for ( int i = 0; i < faceCount && newCount < 256; ++i )
    {
        selbrush_t *brush = g_SelectedFaces.GetAt( i ).brush;
        face_t     *face  = &brush->def->faces[g_SelectedFaces.GetAt( i ).index];
        brush_t    *nb    = PMESH_58( face, brush );
        if ( nb )
            newBrushes[newCount++] = (selbrush_t *)nb;
    }

    g_nUpdateBits = -1;
    Select_Deselect( 1 );

    // relink the new brushes into the selection (Brush_RemoveFromList + Brush_AddToList2).
    for ( int i = 0; i < newCount; ++i )
    {
        Brush_RemoveFromList( newBrushes[i] );
        if ( newBrushes[i]->next || newBrushes[i]->prev )
            Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
        Brush_AddToList2( newBrushes[i] );
    }

    // undo id-stamp tail (identical idiom to the other cluster ops).
    if ( g_lastundo && !g_lastundo->done )
    {
        for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
        {
            i->def->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
            entity_s *owner = (entity_s *)(intptr_t)i->def->owner;
            if ( *(int *)&owner->eclass->fixedsize )
                owner->epairEdits = g_lastundo->id;
        }
    }
    Undo_End();
}

// Legacy direct-invalidate entry, kept for the input handlers that call it - routed through
// the real Sys_UpdateWindows broadcast.
void Ed_InvalidateAllViews()
{
    extern void Sys_UpdateWindows( int bits );   // win_qe3.cpp
    Sys_UpdateWindows( W_ALL );
    if ( g_pParentWnd )
        g_pParentWnd->RoutineProcessing();        // flush now (keeps drags responsive)
}

// ── ConfirmModified (IDB 0x49a030) — the unsaved-changes "Save changes?" prompt ──
// The binary's body is a single MessageBoxA with two shapes selected by the "DefaultSaveNo"
// pref (g_PrefsDlg->loose_changes):
//   loose_changes set  -> "Lose changes?"       MB_OKCANCEL         (IDOK -> discard)
//   loose_changes clear-> "Save changes first?" MB_YESNOCANCEL|ICONEXCLAMATION
//        IDYES -> save first (SaveAsDialog if untitled, else Map_SaveFile), proceed
//        IDNO  -> discard, proceed;  IDCANCEL -> abort (return false)
// ConfirmModified_Decide is the UI-INDEPENDENT decision core so the headless
// `confirmmodified` gate can drive every MessageBox outcome deterministically; ConfirmModified
// pops the box, runs the save, and delegates
// the proceed/abort verdict to the core.  Keeps the GUI and gate on one source of truth.
enum { CM_ABORT = 0, CM_PROCEED = 1, CM_SAVE_THEN_PROCEED = 2 };

// answer: for loose_changes -> MessageBox IDOK/IDCANCEL; else IDYES/IDNO/IDCANCEL.
int ConfirmModified_Decide( int answer, bool looseChanges )
{
    if ( looseChanges )
        return ( answer == IDOK ) ? CM_PROCEED : CM_ABORT;
    // "Save changes first?" — YES=save+proceed, NO=proceed, CANCEL/other=abort.
    if ( answer == IDYES )
        return CM_SAVE_THEN_PROCEED;
    if ( answer == IDNO )
        return CM_PROCEED;
    return CM_ABORT;
}

bool CMainFrame::ConfirmModified()
{
    const bool looseChanges = ( g_PrefsDlg && g_PrefsDlg->loose_changes ) ? true : false;

    int answer;
    if ( looseChanges )
        answer = ::MessageBoxA( ::GetActiveWindow(), "Lose changes?", "Radiant", MB_OKCANCEL );
    else
        answer = ::MessageBoxA( ::GetActiveWindow(), "Save changes first?", "Radiant",
                                MB_YESNOCANCEL | MB_ICONEXCLAMATION );

    int verdict = ConfirmModified_Decide( answer, looseChanges );
    if ( verdict == CM_SAVE_THEN_PROCEED )
    {
        // Binary: strcmp(currentmap,"unnamed.map") → SaveAsDialog; else Map_SaveFile(currentmap).
        // Port adaptation: the active path lives in s_currentMapPath (empty == untitled),
        // so this mirrors OnFileSave's branch exactly.
        if ( !s_currentMapPath[0] )
            return OnFileSaveAs_Confirmed();   // SaveAsDialog(0): false if the user cancels Save-As
        Map_SaveFile( s_currentMapPath, 0, 0 );
        Radiant_FL_Log( "ConfirmModified: saved %s", s_currentMapPath );
        return true;
    }
    return ( verdict == CM_PROCEED );
}

// Radiant_OkToDiscard — the EXACT guard the binary places at the head of every
// destroy-the-map command (OnFileNew/OnFileOpen/OnClose/DoMru):
//   ( !HasUnsavedChangesOrInsidePrefab()
//     && CheckLayeredMaterial_Modifications(...) == lyrMtlGlob_crcToken )  ||  ConfirmModified()
// i.e. proceed silently when nothing is dirty, otherwise prompt.  The layered-material
// CRC half matches ErrorLog_01's already-shipped guard (errorfile.cpp).
extern int      modified;                 // map.cpp 0x23f179c
extern int      prefabStackLevel;         // map.cpp 0x25d5b34
extern unsigned int CheckLayeredMaterial_Modifications( uint8_t *a1, int a2, int a3 ); // layeredmaterials.cpp

static bool HasUnsavedChangesOrInsidePrefab_mf()   // mirror of errorfile.cpp's static (0x489d90)
{
    if ( modified )
        return true;
    if ( prefabStackLevel > 0 )
    {
        prefabLevel_t *p = g_prefabStack;
        for ( int n = 0; ; ++p )
        {
            if ( p->modified )
                return true;
            if ( ++n >= prefabStackLevel )
                return false;
        }
    }
    return false;
}

bool CMainFrame::OkToDiscard()
{
    if ( !HasUnsavedChangesOrInsidePrefab_mf()
         && CheckLayeredMaterial_Modifications( lyrMtlGlob.Layers,
                                                84 * lyrMtlGlob.entryCount, 0 ) == (unsigned)lyrMtlGlob.crcToken )
        return true;                       // nothing dirty → proceed silently
    return ConfirmModified();              // dirty → prompt
}

// OnFileSaveAs that reports whether the user actually saved (vs cancelling the Save-As
// dialog) — ConfirmModified's IDYES-on-untitled path needs the cancel signal.
bool CMainFrame::OnFileSaveAs_Confirmed()
{
    char file[MAX_PATH];
    _snprintf( file, sizeof( file ), "%s", s_currentMapPath );
    OPENFILENAMEA ofn;
    memset( &ofn, 0, sizeof( ofn ) );
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner   = m_hWnd;
    ofn.lpstrFilter = "Map files (*.map)\0*.map\0All files (*.*)\0*.*\0";
    ofn.lpstrFile   = file;
    ofn.nMaxFile    = sizeof( file );
    ofn.lpstrTitle  = "Save Map As";
    ofn.lpstrDefExt = "map";
    ofn.Flags       = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if ( !GetSaveFileNameA( &ofn ) )
        return false;                      // user cancelled Save-As → abort the parent op

    // SaveAsDialog (0x49A760) .map path: after the extension default, copy the chosen path
    // into currentmap, bump the MRU (MRU_NewItem promotes it to slot 0), and refresh the
    // File-menu recent-files list — verbatim from 0x49a800-0x49a84a — BEFORE Map_SaveFile.
    _snprintf( s_currentMapPath, sizeof( s_currentMapPath ), "%s", file );
    if ( g_qeglobals.d_lpMruMenu )
    {
        MRU_NewItem( g_qeglobals.d_lpMruMenu, file );
        MRU_InsertItem( g_qeglobals.d_lpMruMenu, GetSubMenu( GetMenu()->GetSafeHmenu(), 0 ) );
    }

    Map_SaveFile( file, 0, 0 );
    char title[MAX_PATH + 32];
    _snprintf( title, sizeof( title ), "CoD4Radiant - %s", file );
    SetWindowText( title );
    return true;
}

// CXYWnd clipboard ops.  KISAK: the binary mirrors the copied selection to the Win32 OLE
// clipboard (RegisterClipboardFormatA("RadiantClippings")) so brushes can be pasted between
// two running editors; that OLE + CMemFile layer is not ported.  These bodies use the in-app
// clipboard buffer (entity.cpp), which is the full single-editor round trip (Copy ->
// Entity_WriteSelected_R, Paste -> Map_ImportBuffer) and also carries the selection between
// maps (Map_Free's "Copy selection?" box on File->New/Open).
extern void RadiantClipboard_Copy();    // entity.cpp
extern void RadiantClipboard_Paste();   // entity.cpp

void CXYWnd::Paste()
{
    RadiantClipboard_Paste();
}

void CXYWnd::Copy()
{
    RadiantClipboard_Copy();
}

// CXYWnd::PositionView (0x46DE10) — recenter this XY view's origin.  The two in-plane
// axes depend on the view type (ED_VIEW_YZ=0/XZ=1/XY=2): nDim1 = (view==YZ), nDim2 =
// (view!=XY)+1 → XY:{0,1} XZ:{0,2} YZ:{1,2}.  If a point-edit mode is active and there
// are move points, center on their average; otherwise center on the camera origin, and
// if exactly one brush is selected, on that brush's bbox centre.  Faithful to the disasm.
void CXYWnd::PositionView()
{
    int nDim1 = ( m_nViewType == ED_VIEW_YZ ) ? 1 : 0;
    int nDim2 = ( m_nViewType != ED_VIEW_XY ) + 1;

    if ( ( g_qeglobals.d_select_mode == sel_vertex
        || g_qeglobals.d_select_mode == sel_curvepoint
        || g_qeglobals.d_select_mode == sel_area
        || g_qeglobals.d_select_mode == sel_terrainpoint )
      && g_qeglobals.d_num_move_points )
    {
        m_vOrigin[nDim1] = 0.0f;
        m_vOrigin[nDim2] = 0.0f;
        for ( int p = 0; p < g_qeglobals.d_num_move_points; ++p )
        {
            m_vOrigin[nDim1] += g_qeglobals.d_move_points[p]->xyz[nDim1];
            m_vOrigin[nDim2] += g_qeglobals.d_move_points[p]->xyz[nDim2];
        }
        m_vOrigin[nDim1] = (float)( m_vOrigin[nDim1] / (double)g_qeglobals.d_num_move_points );
        m_vOrigin[nDim2] = (float)( m_vOrigin[nDim2] / (double)g_qeglobals.d_num_move_points );
    }
    else
    {
        CCamWnd *cam = g_pParentWnd->m_pCamWnd;
        m_vOrigin[nDim1] = cam->camera.origin[nDim1];
        m_vOrigin[nDim2] = cam->camera.origin[nDim2];
        for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
        {
            // The binary tests the LAST node (i->next == sentinel) — center on the single
            // (or last) selected brush's bbox.
            if ( i->next == &selected_brushes )
            {
                m_vOrigin[nDim1] = ( i->def->maxs[nDim1] + i->def->mins[nDim1] ) * 0.5f;
                m_vOrigin[nDim2] = ( i->def->maxs[nDim2] + i->def->mins[nDim2] ) * 0.5f;
            }
        }
    }
}

// ─── CXYWnd::SetViewType (0x46DF90) — set the 2D-view axis (YZ/XZ/XY) ─────────────
// The binary stores the EViewType (== m_nViewType value) directly, then — only in the
// floating single-XY layout (m_nCurrentStyle==1) — retitles the window "YZ Side" /
// "XZ Front" / "XY Top".  In the port's docked layout (m_nCurrentStyle!=1 by default)
// the title update is skipped exactly as the binary skips it.
void CXYWnd::SetViewType( EViewType vt )
{
    m_nViewType = (int)vt;
    if ( g_pParentWnd->m_nCurrentStyle == 1 )
    {
        const char *title = "YZ Side";
        if ( m_nViewType == 2 )      title = "XY Top";
        else if ( m_nViewType == 1 ) title = "XZ Front";
        SetWindowTextA( title );
    }
}

// ─── CXYWnd::SetRotateMode (0x46E090) — enter/leave free mouse-rotation mode ──────
// Toolbar "Free rotation" button (OnSelectMouserotate).  Turning it ON requires a
// selection (else prints + stays off) and seats the rotate pivot at the selection
// centre (Select_GetTrueMid → g_vRotateOrigin), zeroing the rotation accumulator.
// Returns the resulting g_bRotateMode so the caller can light the toolbar button.
extern void     Select_GetTrueMid( float *center );   // select.cpp (0x48FC20)
extern float    g_vRotateOrigin[3];                   // drag.cpp   (0x23F1658)
extern float    g_vRotation[3];                       // drag.cpp   (0x23F164C)
extern bool     g_bRotateMode;                        // drag.cpp   (0x23F16D9)

bool CXYWnd::SetRotateMode( char bMode )
{
    if ( bMode && selected_brushes.next != &selected_brushes )
    {
        g_bRotateMode = true;
        Select_GetTrueMid( g_vRotateOrigin );
        g_vRotation[0] = g_vRotation[1] = g_vRotation[2] = 0.0f;
    }
    else
    {
        if ( bMode )
            Sys_Printf( "Need a brush selected to turn on Mouse Rotation mode\n" );
        g_bRotateMode = false;
    }
    RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
    return g_bRotateMode;
}

// CTextureBar::GetSurfaceAttributes + the rest of the bar now live in texturebar.cpp.

// ═════════════════════════════════════════════════════════════════════════════════
// UI COMMAND-WIRING batch - CMainFrame WM_COMMAND thunks over already-ported cores, each
// transcribed verbatim from its binary handler (EA in the trailing comment).
extern int g_bCrossHairs;   // engine_stubs.cpp (0x25D5B06) — XY crosshair toggle

// ── Camera fly keys (arrows / , . ) — Left/Right pitch-yaw, Forward/Back/Strafe move.
//    Each first tries PMESH_10 (patch-vertex move in vertex/curvepoint mode); if that
//    handles it (returns nonzero) the camera is left alone.  IDB 0x426720/70/0x4266B0/
//    0x426610/0x4267C0/0x426820.
extern char PMESH_10( char bAdd, int a2, int a3 );   // pmesh.cpp 0x43CB80

void CMainFrame::OnCameraLeft()             // 0x426720 (cmd 33057, Left)
{
    if ( !PMESH_10( 0, 1, 0 ) )
    {
        m_pCamWnd->camera.angles[1] += 22.5f;
        g_nUpdateBits |= 2 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
    }
}

void CMainFrame::OnCameraRight()            // 0x426770 (cmd 33058, Right)
{
    if ( !PMESH_10( 0, -1, 0 ) )
    {
        m_pCamWnd->camera.angles[1] -= 22.5f;
        g_nUpdateBits |= 2 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
    }
}

void CMainFrame::OnCameraForward()          // 0x4266B0 (cmd 33059, Up)
{
    if ( !PMESH_10( 0, 0, -1 ) )
    {
        m_pCamWnd->camera.origin[0] += m_pCamWnd->camera.forward[0] * 32.0f;
        m_pCamWnd->camera.origin[1] += m_pCamWnd->camera.forward[1] * 32.0f;
        m_pCamWnd->camera.origin[2] += 32.0f * m_pCamWnd->camera.forward[2];
        g_nUpdateBits |= 2 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
    }
}

void CMainFrame::OnCameraBack()             // 0x426610 (cmd 33060, Down)
{
    if ( !PMESH_10( 0, 0, 1 ) )
    {
        m_pCamWnd->camera.origin[0] -= m_pCamWnd->camera.forward[0] * 32.0f;
        m_pCamWnd->camera.origin[1] -= m_pCamWnd->camera.forward[1] * 32.0f;
        m_pCamWnd->camera.origin[2] -= 32.0f * m_pCamWnd->camera.forward[2];
        g_nUpdateBits |= 2 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
    }
}

void CMainFrame::OnCameraStrafeleft()       // 0x4267C0 (cmd 33063, ',')
{
    m_pCamWnd->camera.origin[0] -= m_pCamWnd->camera.right[0] * 32.0f;
    m_pCamWnd->camera.origin[1] -= m_pCamWnd->camera.right[1] * 32.0f;
    m_pCamWnd->camera.origin[2] -= 32.0f * m_pCamWnd->camera.right[2];
    g_nUpdateBits |= 2 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
}

void CMainFrame::OnCameraStraferight()      // 0x426820 (cmd 33064, '.')
{
    m_pCamWnd->camera.origin[0] += m_pCamWnd->camera.right[0] * 32.0f;
    m_pCamWnd->camera.origin[1] += m_pCamWnd->camera.right[1] * 32.0f;
    m_pCamWnd->camera.origin[2] += 32.0f * m_pCamWnd->camera.right[2];
    g_nUpdateBits |= 2 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
}

// ── Grid toggle (0) — flip d_showgrid, redraw XY+Z.  IDB 0x426930. ──────────────────
void CMainFrame::OnGridToggle()             // cmd 33065
{
    g_qeglobals.d_showgrid = !g_qeglobals.d_showgrid;
    g_nUpdateBits |= W_XY | W_Z;
}

// ── Grid size next/prev ( ] / [ ) — step d_gridsize within [0,10].  IDB 0x4289A0/D0. ─
void CMainFrame::OnGridNext()               // cmd 33083 (])
{
    if ( g_qeglobals.d_gridsize < 10 )
        ++g_qeglobals.d_gridsize;
    Radiant_CheckGridMenu( this );
    g_nUpdateBits |= 0xAu;                   // W_XY | W_Z
    SetGridStatus();
}

void CMainFrame::OnGridPrev()               // cmd 33084 ([)
{
    if ( g_qeglobals.d_gridsize > 0 )
        --g_qeglobals.d_gridsize;
    Radiant_CheckGridMenu( this );
    g_nUpdateBits |= 0xAu;
    SetGridStatus();
}

// ── Texture-shift step inc/dec (Shift+KP-/KP+) — walk d_savedinfo.d_gridsize as an INT
//    counter (LODWORD bit-manip; the wrap-guard keeps it out of 0).  IDB 0x4287B0/830.
void CMainFrame::OnSelectionTextureSnapDec()  // cmd 33072
{
    int *g = (int *)&g_qeglobals.d_savedinfo.d_gridsize;
    if ( !--*g )
        *g = -1;   // binary sets NAN bit-pattern (0xFFFFFFFF int) to avoid 0
    SetGridStatus();
}

void CMainFrame::OnSelectionTextureSnapInc()  // cmd 33073
{
    int *g = (int *)&g_qeglobals.d_savedinfo.d_gridsize;
    if ( !++*g )
        *g = 1;
    SetGridStatus();
}

// ── Texture fit / fit-all (Ctrl+F / Ctrl+Shift+F) — Brush_FitTexture 1×1.  IDB
//    0x4287D0 / 0x428800. ─────────────────────────────────────────────────────────────
extern void Brush_FitTexture( float x, float y, int a4 );      // select.cpp 0x4939E0
void CMainFrame::OnSelectionTextureFitUnk()   // cmd 33074
{
    Brush_FitTexture( 1.0f, 1.0f, 0 );
    g_nUpdateBits = -1;
}
void CMainFrame::OnTextureFitAll()            // cmd 33234
{
    Brush_FitTexture( 1.0f, 1.0f, 1 );
    g_nUpdateBits = -1;
}

// ── Texture rotate CW / CCW (Ctrl+arrows) — Brush_RotateTexture(±ClampGridSize).  IDB
//    0x428850 / 0x428860. ─────────────────────────────────────────────────────────────
void CMainFrame::OnTexRotateClockwise()       // cmd 33075
{
    Brush_RotateTexture( ClampGridSize() );
}
void CMainFrame::OnTexRotateCounterCW()       // cmd 33076
{
    Brush_RotateTexture( -ClampGridSize() );
}

// ── Texture shift L/R/U/D (Shift+arrows) — Brush_ShiftTexture by ±2·grid.  IDB
//    0x4288E0/910/870/8A0. ────────────────────────────────────────────────────────────
extern void Brush_ShiftTexture( float ds, float dt );          // select.cpp 0x491F20
void CMainFrame::OnTexShiftLeft()             // cmd 33079
{
    Brush_ShiftTexture( grid_sizes[g_qeglobals.d_gridsize] + grid_sizes[g_qeglobals.d_gridsize], 0.0f );
}
void CMainFrame::OnTexShiftRight()            // cmd 33080
{
    Brush_ShiftTexture( grid_sizes[g_qeglobals.d_gridsize] * -2.0f, 0.0f );
}
void CMainFrame::OnTexShiftUp()               // cmd 33081
{
    Brush_ShiftTexture( 0.0f, grid_sizes[g_qeglobals.d_gridsize] + grid_sizes[g_qeglobals.d_gridsize] );
}
void CMainFrame::OnTexShiftDown()             // cmd 33082
{
    Brush_ShiftTexture( 0.0f, grid_sizes[g_qeglobals.d_gridsize] * -2.0f );
}

// ── Z-view zoom (Ctrl+Del / Ctrl+Ins) + Z 100% ─────────────────────────────────────
void CMainFrame::OnViewZ100()               // cmd 32998 — empty stub in the binary (0x424740)
{
}
void CMainFrame::OnViewZzoomin()            // cmd 32999 (0x424A00)
{
    g_nUpdateBits |= 0x28u;                  // W_Z | W_Z_OVERLAY
    z_scale *= 1.25f;
    if ( z_scale > 160.0f )
        z_scale = 160.0f;
}
void CMainFrame::OnViewZzoomout()           // cmd 33000 (0x424A40)
{
    g_nUpdateBits |= 0x28u;
    z_scale *= 0.800000011920929f;
    if ( z_scale < 0.003125000046566129f )
        z_scale = 0.003125f;
}

// ── Cubic-clip zoom out / in (Ctrl+[ / ]) — ±m_nCubicScale [1,220], persist.  IDB
//    0x428F50 / 0x428F10. ─────────────────────────────────────────────────────────────
void CMainFrame::OnViewCubeout()            // cmd 32819
{
    if ( ++g_PrefsDlg->m_nCubicScale > 220 )
        g_PrefsDlg->m_nCubicScale = 220;
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits |= 1u;
    SetGridStatus();
}
void CMainFrame::OnViewCubein()             // cmd 32820
{
    if ( --g_PrefsDlg->m_nCubicScale < 1 )
        g_PrefsDlg->m_nCubicScale = 1;
    Prefs_SavePrefs( g_PrefsDlg );
    g_nUpdateBits |= 1u;
    SetGridStatus();
}

// ── View layout XY / YZ / XZ (0x424710/0x423FB0/0x424A80) — set the active 2D-view axis.
//    m_nCurrentStyle==2 is the "no XY pane" style; skip there.  EViewType == m_nViewType. ─
void CMainFrame::OnViewXy()                 // cmd 32772
{
    if ( m_nCurrentStyle != 2 )
    {
        m_pXYWnd->SetViewType( CXYWnd::XY );
        m_pXYWnd->PositionView();
    }
    g_nUpdateBits |= W_XY;
}
void CMainFrame::OnViewYz()                 // cmd 32774
{
    if ( m_nCurrentStyle != 2 )
    {
        m_pXYWnd->SetViewType( CXYWnd::YZ );
        m_pXYWnd->PositionView();
    }
    g_nUpdateBits |= W_XY;
}
void CMainFrame::OnViewXz()                 // cmd 32773
{
    if ( m_nCurrentStyle != 2 )
    {
        m_pXYWnd->SetViewType( CXYWnd::XZ );
        m_pXYWnd->PositionView();
    }
    g_nUpdateBits |= W_XY;
}

// ── Toggle YZ / XZ view (0x427230/0x427220) — EMPTY stubs in the binary. ────────────
void CMainFrame::OnToggleviewYz() {}        // cmd 32797
void CMainFrame::OnToggleviewXz() {}        // cmd 32798

// ── View→Toggle→{Console,Camera,XY,Z} show/hide (0x426A90 / 0x426A40 / 0x426AE0 /
//    0x426B30) — the four SHOW/HIDE toggles (dead in the port until the U7 backfill).
//    Each is the same shape: gate on the layout style, then flip the child window's
//    visibility with IsWindowVisible + ShowWindow(SW_HIDE/SW_SHOW).  Ported verbatim.
//    NOTE the STYLE GATES differ per handler and are the binary's, not a typo:
//      console/camera : style > 0 && style < 3   (i.e. 1 or 2)
//      XY             : style == 1  ONLY
//      Z              : style == 1 || style == 2, ELSE tail-jumps to Undo_Redo (see below)
void CMainFrame::OnToggleconsole()          // cmd 33068 (0x426A90)
{
    // The binary's m_pEditWnd is the docked console CEdit (g_qeglobals.d_hwndEdit); in this
    // port that is the embedded m_wndConsole child.
    if ( m_nCurrentStyle > 0 && m_nCurrentStyle < 3 )
    {
        if ( m_wndConsole.m_hWnd )
        {
            if ( ::IsWindowVisible( m_wndConsole.m_hWnd ) )
                m_wndConsole.ShowWindow( SW_HIDE );
            else
                m_wndConsole.ShowWindow( SW_SHOW );
        }
    }
}

void CMainFrame::OnTogglecamera()           // cmd 33069 (0x426A40), Shift+Ctrl+C
{
    if ( m_nCurrentStyle > 0 && m_nCurrentStyle < 3 )
    {
        if ( m_pCamWnd && m_pCamWnd->m_hWnd )
        {
            if ( ::IsWindowVisible( m_pCamWnd->m_hWnd ) )
                m_pCamWnd->ShowWindow( SW_HIDE );
            else
                m_pCamWnd->ShowWindow( SW_SHOW );
        }
    }
}

void CMainFrame::OnToggleview()             // cmd 33071 (0x426AE0), Shift+Ctrl+V
{
    if ( m_nCurrentStyle == 1 )
    {
        if ( m_pXYWnd && m_pXYWnd->m_hWnd )
        {
            if ( ::IsWindowVisible( m_pXYWnd->m_hWnd ) )
                m_pXYWnd->ShowWindow( SW_HIDE );
            else
                m_pXYWnd->ShowWindow( SW_SHOW );
        }
    }
}

void CMainFrame::OnTogglez()                // cmd 33070 (0x426B30), Shift+Ctrl+Z
{
    // ORIGINAL QUIRK, reproduced verbatim: the style-mismatch path at 0x426B43 is
    // `pop esi ; jmp Undo_Redo` — i.e. in any layout other than 1/2 this command performs
    // a REDO instead of a Z-view toggle.  Not an ICF artifact (the else arm is reachable by
    // fall-through from the two `jz`s and the function already has its own `pop esi ; retn`
    // epilogue at 0x426B7D).  This port fixes m_nCurrentStyle at 1, so the arm is unreachable
    // here; it is kept so the code matches the binary.
    if ( m_nCurrentStyle == 1 || m_nCurrentStyle == 2 )
    {
        if ( m_pZWnd && m_pZWnd->m_hWnd )
        {
            if ( ::IsWindowVisible( m_pZWnd->m_hWnd ) )
                m_pZWnd->ShowWindow( SW_HIDE );
            else
                m_pZWnd->ShowWindow( SW_SHOW );
        }
    }
    else
    {
        Undo_Redo();
    }
}

// ── Next view (Ctrl+Tab, 0x426DB0) — cycle the active XY view type XY→XZ→YZ→XY. ──────
void CMainFrame::OnViewNextview()           // cmd 32789
{
    if ( m_nCurrentStyle != 2 )
    {
        int vt = m_pXYWnd->m_nViewType;
        if ( vt == 2 )      m_pXYWnd->SetViewType( CXYWnd::XZ );
        else if ( vt == 1 ) m_pXYWnd->SetViewType( CXYWnd::YZ );
        else                m_pXYWnd->SetViewType( CXYWnd::XY );
        m_pXYWnd->PositionView();
        g_nUpdateBits |= 2u;
    }
}

// ── Toolbar Main / Texture show-hide (0x4290F0/0x429100) — EMPTY stubs in the binary. ─
void CMainFrame::OnToolbarMain() {}         // cmd 32830
void CMainFrame::OnToolbarTexture() {}      // cmd 32832

// ── Center 2D on camera (Shift+C, 0x42A2D0) — copy the camera origin into the XY view. ─
void CMainFrame::OnCenter2DOnCamera()       // cmd 33108
{
    CXYWnd  *xy  = g_pParentWnd->m_pActiveXY;
    CCamWnd *cam = g_pParentWnd->m_pCamWnd;
    g_nUpdateBits |= 2u;
    xy->m_vOrigin[0] = cam->camera.origin[0];
    xy->m_vOrigin[1] = cam->camera.origin[1];
    xy->m_vOrigin[2] = cam->camera.origin[2];
}

// ── Selection cycle next / prev (Shift+, / Shift+.) — SelectNext/Prev + redraw. ──────
extern void SelectNext();                   // select.cpp 0x494210
extern void SelectPrev();                   // select.cpp 0x494360
void CMainFrame::OnSelectNext()             // cmd 33160 (0x423C90)
{
    SelectNext();
    g_nUpdateBits |= 5u;                     // W_XY | W_CAMERA
}
void CMainFrame::OnSelectPrev()             // cmd 33161 (0x423CA0)
{
    SelectPrev();
    g_nUpdateBits |= 5u;
}

// ── Move selection up / down (KP+ / KP-) — translate ±grid along Z, wrapped in undo. ──
extern void Undo_ClearRedo();               // undo.cpp
extern void Undo_GeneralStart( const char *op );
extern void Undo_AddBrushList( selbrush_t *list );
extern void Undo_EndBrushList( selbrush_t *list );
extern void Undo_End();
void CMainFrame::OnSelectionMovedown()      // cmd 32829 (0x429050)
{
    extern void Select_Move( const float *delta, char bSnap );
    Undo_ClearRedo();
    Undo_GeneralStart( "move up" );
    Undo_AddBrushList( &selected_brushes );
    float vAmt[3] = { 0.0f, 0.0f, -grid_sizes[g_qeglobals.d_gridsize] };
    Select_Move( vAmt, 1 );
    g_nUpdateBits |= 0xBu;                    // W_XY | W_CAMERA | W_Z
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}
void CMainFrame::OnSelectionMoveup()        // cmd 32831 (0x4290B0)
{
    extern void Select_Move( const float *delta, char bSnap );
    float delta[3] = { 0.0f, 0.0f, grid_sizes[g_qeglobals.d_gridsize] };
    Select_Move( delta, 1 );
    g_nUpdateBits |= 0xBu;
}

// ── Selection nudge L/R/U/D (Alt+arrows) — NudgeSelection(dir, this, grid).  IDB
//    0x429510/530/550/4F0. ────────────────────────────────────────────────────────────
extern void NudgeSelection( int dir, CMainFrame *frame, float amt );   // select.cpp 0x429570
void CMainFrame::OnSelectionSelectNudgeleft()  // cmd 32847
{
    NudgeSelection( 0, this, grid_sizes[g_qeglobals.d_gridsize] );
}
void CMainFrame::OnSelectionSelectNudgeright() // cmd 32848
{
    NudgeSelection( 2, this, grid_sizes[g_qeglobals.d_gridsize] );
}
void CMainFrame::OnSelectionSelectNudgeup()    // cmd 32849
{
    NudgeSelection( 1, this, grid_sizes[g_qeglobals.d_gridsize] );
}
void CMainFrame::OnSelectionSelectNudgedown()  // cmd 32850
{
    NudgeSelection( 3, this, grid_sizes[g_qeglobals.d_gridsize] );
}

// ── Splay / Set View To Entity / Link Selected / Get Distance / Select All Of Type ──
extern void DoSplay();                      // pmesh.cpp 0x44A430
extern void SetViewToEntity();              // select.cpp 0x48C3D0
extern void LinkSelected();                 // select.cpp 0x48C7B0
extern void Get_DistanceBetweenEnts();      // select.cpp 0x47A260
extern void Select_ByTexture( int recurse );      // select.cpp 0x4934F0
extern void Select_ByClassSimilar();              // select.cpp 0x493830
void CMainFrame::OnSplay()                  // cmd 33157 (0x4254F0)
{
    DoSplay();
}
void CMainFrame::OnSetViewToEntity()        // cmd 33210 (0x425540)
{
    SetViewToEntity();
}
void CMainFrame::OnLinkSelected()           // cmd 33211 (0x425500)
{
    LinkSelected();
}
void CMainFrame::OnDistanceBetweenEntities() // cmd 33178 (0x4294D0)
{
    Get_DistanceBetweenEnts();
}
void CMainFrame::OnSelectAllOfType()        // cmd 33093 (0x42B470)
{
    if ( selected_brushes.next == &selected_brushes )
        Select_ByTexture( 0 );
    else
        Select_ByClassSimilar();
    g_nUpdateBits = -1;
}
void CMainFrame::OnSelectAllOfTypeRecursive() // cmd 33212 (0x42B4B0)
{
    if ( selected_brushes.next == &selected_brushes )
        Select_ByTexture( 1 );
    else
        Select_ByClassSimilar();
    g_nUpdateBits = -1;
}

// ── Hide/Show workflow (H / Shift+H / Alt+H / Ctrl+H) — select.cpp cores. ────────────
extern void Select_Hide();                  // select.cpp 0x493D50
extern void Select_HideUnselected();        // select.cpp 0x493DD0
extern void ShowHidden();                   // select.cpp 0x493E50
extern void ShowLastHidden();               // select.cpp 0x493EA0
void CMainFrame::OnHideSelected()           // cmd 32923 (0x42B6A0)
{
    Select_Hide();
    Select_Deselect( 1 );
}
void CMainFrame::OnHideUnselected()         // cmd 32934 (0x42B6C0)
{
    Select_HideUnselected();
}
void CMainFrame::OnShowHidden()             // cmd 32924 (0x42B6D0)
{
    ShowHidden();
}
void CMainFrame::OnShowLastHidden()         // cmd 33246 (0x42B6E0)
{
    ShowLastHidden();
}

// ── Selection draw toggles: crosshair / no-outline / no-tint.  IDB 0x42B690/0x425630/50.
void CMainFrame::OnViewCrosshair()          // cmd 33100
{
    g_bCrossHairs ^= 1u;
    g_nUpdateBits |= 2u;
}
void CMainFrame::OnSelectionNoOutline()     // cmd 33103
{
    g_qeglobals.dontDrawSelectedOutlines = !g_qeglobals.dontDrawSelectedOutlines;
    g_nUpdateBits = -1;
}
void CMainFrame::OnSelectionNoTint()        // cmd 33172
{
    g_qeglobals.dontDrawSelectedTint = !g_qeglobals.dontDrawSelectedTint;
    g_nUpdateBits = -1;
}

// ── Select same target / targetname (B / Ctrl+B) — copy the FIRST selected entity's
//    key value onto all selected.  IDB 0x42ADA0 (targetname) / 0x42ADD0 (target). ──────
extern void Select_AllByKeyValue( const char *key );   // select.cpp (sub_485B70 0x485B70)
void CMainFrame::OnSelectSameTargetname()   // cmd 36121 (sub_42ADA0)
{
    Select_AllByKeyValue( "targetname" );
    g_nUpdateBits = -1;
}
void CMainFrame::OnSelectSameTarget()       // cmd 36123 (sub_42ADD0)
{
    Select_AllByKeyValue( "target" );
    g_nUpdateBits = -1;
}

// ── ON_UPDATE_COMMAND_UI handlers — keep menu items' enable/grey state live. ─────────
extern undo_s *g_lastundo;              // undo.cpp (0x23F162C)
extern bool    Undo_RedoAvailable();    // undo.cpp — g_lastredo != nullptr
extern int     g_region_active;         // map.cpp (0x23F1744)

void CMainFrame::OnUpdateViewCameraupdate( CCmdUI *pCmdUI )  // 0x4264B0 (32782)
{
    pCmdUI->Enable( !m_bCamPreview );
}
void CMainFrame::OnUpdateEditUndo( CCmdUI *pCmdUI )          // 0x428750 (57643)
{
    pCmdUI->Enable( g_lastundo && g_lastundo->done );
}
void CMainFrame::OnUpdateEditRedo( CCmdUI *pCmdUI )          // 0x428790 (57644)
{
    pCmdUI->Enable( Undo_RedoAvailable() );
}
void CMainFrame::OnUpdateFileSaveregion( CCmdUI *pCmdUI )    // 0x429030 (32827)
{
    pCmdUI->Enable( g_region_active );
}

// ── Light-preview submenu (F8 workflow) ─────────────────────────────────────────────
extern int  CCamWnd_AddLightPreview( CCamWnd *cam, selbrush_t *inst, int arg2, const orientation_t *orient ); // camwnd.cpp 0x406200
extern int  CCamWnd_RemoveLightPreview( selbrush_t *removed, CCamWnd *cam );  // camwnd.cpp 0x4062D0
extern float world_orient_matrix[4][3];                                       // entity.cpp 0x6DE290

void CMainFrame::OnEnableLightPreview()     // cmd 33950 (0x4240C0)
{
    g_PrefsDlg->enable_light_preview = ( g_PrefsDlg->enable_light_preview == 0 );
    Prefs_SavePrefs( g_PrefsDlg );
    CheckMenuItem( GetMenu()->GetSafeHmenu(), 33950, g_PrefsDlg->enable_light_preview ? MF_CHECKED : MF_UNCHECKED );
    g_nUpdateBits |= 1u;
}
void CMainFrame::OnPreviewSun()             // cmd 36108 (0x424060)
{
    g_PrefsDlg->preview_sun_aswell = ( g_PrefsDlg->preview_sun_aswell == 0 );
    Prefs_SavePrefs( g_PrefsDlg );
    CheckMenuItem( GetMenu()->GetSafeHmenu(), 36108, g_PrefsDlg->preview_sun_aswell ? MF_CHECKED : MF_UNCHECKED );
    g_nUpdateBits |= W_CAMERA;
}
void CMainFrame::OnStartPreviewSelected()   // cmd 33951 (0x424120)
{
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        entity_s *owner = i->owner;
        if ( ( ((entity_s_def *)owner->def)->eclass->classtype & 1 ) != 0 )   // light
            CCamWnd_AddLightPreview( m_pCamWnd, i, 0, (const orientation_t *)world_orient_matrix );
    }
    g_nUpdateBits |= W_CAMERA;
}
void CMainFrame::OnStopPreviewSelected()    // cmd 33952 (0x424170)
{
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        entity_s *owner = i->owner;
        if ( ( ((entity_s_def *)owner->def)->eclass->classtype & 1 ) != 0 )   // light
            CCamWnd_RemoveLightPreview( i, m_pCamWnd );
    }
    g_nUpdateBits |= 1u;
}
void CMainFrame::OnClearPreviewList()       // cmd 33953 (0x4241C0)
{
    g_nUpdateBits |= W_CAMERA;
    m_pCamWnd->light_preview_count = 0;
}
void CMainFrame::OnPreviewAtMaxIntensity()  // cmd 36122 (0x425670)
{
    g_qeglobals.preview_at_max_intensity = !g_qeglobals.preview_at_max_intensity;
    g_nUpdateBits = -1;
}

// ── File→Load / Save-Selected / Save-Region ─────────────────────────────────────────
extern void Map_ImportFile( const char *path );                  // map.cpp 0x488C70
extern void DefaultExtension( char *path, const char *ext );     // cmdlib.cpp
extern void Map_SaveFile( const char *path, char bRegion, char a3 ); // map.cpp
typedef int WriteFunc_map_t( int ctx, const char *fmt, ... );
extern void Entity_WriteSelected_R( WriteFunc_map_t **writer );  // map.cpp 0x488DF0

// File-writing WriteFunc for OnFileExportmap_Sub (a 2-slot writer whose [1] is the FILE*).
static int Radiant_FileWriter( int ctx, const char *fmt, ... )
{
    WriteFunc_map_t **writer = (WriteFunc_map_t **)ctx;
    FILE *fp = (FILE *)writer[1];
    va_list ap; va_start( ap, fmt );
    int n = vfprintf( fp, fmt, ap );
    va_end( ap );
    return n;
}

// 0x488EB0  OnFileExportmap_Sub — write the selection to `pFilename` (iwmap 4 header).
static void OnFileExportmap_Sub( const char *pFilename )
{
    FILE *fp = fopen( pFilename, "w" );
    if ( !fp )
    {
        Sys_Printf( "ERROR!!!! Couldn't open %s\n", pFilename );
        return;
    }
    fprintf( fp, "iwmap %i\n", 4 );
    WriteFunc_map_t *writer[2];
    writer[0] = &Radiant_FileWriter;
    writer[1] = (WriteFunc_map_t *)fp;
    Entity_WriteSelected_R( writer );
    fclose( fp );
}

void CMainFrame::OnFileImportmap()          // cmd 32844 — File→Load (merge .map)  (0x429290)
{
    char file[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    memset( &ofn, 0, sizeof( ofn ) );
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner   = m_hWnd;
    ofn.lpstrFilter = "Map files (*.map)\0*.map\0";
    ofn.lpstrFile   = file;
    ofn.nMaxFile    = sizeof( file );
    ofn.lpstrTitle  = "Load Map";
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if ( GetOpenFileNameA( &ofn ) )
        Map_ImportFile( file );
}

void CMainFrame::OnFileExportmap()          // cmd 32845 — File→Save Selected  (0x4293A0)
{
    char file[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    memset( &ofn, 0, sizeof( ofn ) );
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner   = m_hWnd;
    ofn.lpstrFilter = "Map files (*.map)\0*.map\0";
    ofn.lpstrFile   = file;
    ofn.nMaxFile    = sizeof( file );
    ofn.lpstrDefExt = "map";
    ofn.lpstrTitle  = "Save Selected";
    ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    if ( GetSaveFileNameA( &ofn ) )
        OnFileExportmap_Sub( file );
}

void CMainFrame::OnFileSaveregion()         // cmd 32827 — File→Save Region  (0x429020)
{
    // The binary calls SaveAsDialog(1); its region path (a1!=0) is just the Save dialog +
    // DefaultExtension(".reg") + Map_SaveFile(file, 1, 0) — the MRU management (MRU_* not
    // ported) only runs on the non-region (SaveAs) path, so it is faithfully omitted here.
    char file[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    memset( &ofn, 0, sizeof( ofn ) );
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner   = g_qeglobals.d_hwndCamera;
    ofn.lpstrFilter = "Map file (*.map, *.reg)\0*.map\0*.reg\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile   = file;
    ofn.nMaxFile    = sizeof( file );
    ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST; // binary Flags 6162
    if ( !GetSaveFileNameA( &ofn ) )
        return;
    DefaultExtension( file, ".reg" );
    Map_SaveFile( file, 1, 0 );
}

// ── CEdBlankPane (Tex placeholder + laymat content render shell) ──────────────────
extern bool g_radiantFirstLightRendererReady;

BEGIN_MESSAGE_MAP( CEdBlankPane, CWnd )
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CEdBlankPane::CEdBlankPane() {}

BOOL CEdBlankPane::PreCreateWindow( CREATESTRUCT& cs )
{
    cs.lpszClass = AfxRegisterWndClass(
        CS_OWNDC | CS_HREDRAW | CS_VREDRAW, ::LoadCursor( NULL, IDC_ARROW ), NULL, NULL );
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    return CWnd::PreCreateWindow( cs );
}

int CEdBlankPane::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    CRect rc; GetClientRect( &rc );
    m_nWidth = rc.Width(); m_nHeight = rc.Height();
    return 0;
}

void CEdBlankPane::OnSize( UINT nType, int cx, int cy )
{
    CWnd::OnSize( nType, cx, cy );
    m_nWidth = cx; m_nHeight = cy;
    if ( dx.device && cx > 0 && cy > 0 )
        R_Hwnd_Resize( (HWND__ *)GetSafeHwnd(), cx, cy );
}

BOOL CEdBlankPane::OnEraseBkgnd( CDC* /*pDC*/ )
{
    return TRUE;
}

void CEdBlankPane::OnPaint()
{
    CPaintDC dc( this );
    if ( !dx.device )
        return;
    HWND__ *hwnd = (HWND__ *)GetSafeHwnd();
    if ( !R_SetupRendertarget_CheckDevice( hwnd ) )
        return;
    R_BeginFrame();
    R_BeginSharedCmdList();
    R_AddCmdClearScreen( 7, m_clear, 1.0f, 0 );
    R_EndFrame();
    R_IssueRenderCommands( (uint32_t)-1 );
    R_SortMaterials();
    R_CheckTargetWindow( hwnd );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x42c030  MainFrm_BrushList  (MainFrm.cpp:6372-6384)
// Not a window populate: a pure Assert-based integrity walk over one brush list,
// called by Map_LoadFile / the prefab enter-leave pair / Undo_GeneralStart+Undo_End
// with a label describing the moment.  Walks the DISPLAY list (prev/next) and checks
// the link back-pointers, the def/owner cross-link and the def refcount.
// (Its sibling MainFrm_EntList 0x42c1e0 is deliberately still a stub — see engine_stubs.cpp.)
// ─────────────────────────────────────────────────────────────────────────────
extern void Assert( const char *file, int line, int type, const char *fmt, ... );  // engine_stubs.cpp

void MainFrm_BrushList( int message, selbrush_t *brushList )
{
    const char *msg = (const char *)(intptr_t)message;
    iassert( brushList );   // mainfrm.cpp:6372

    for ( selbrush_t *brush = brushList->next; brush != brushList; brush = brush->next )
    {
        vassert( (brush), "(message) = %s", msg );   // MainFrm.cpp:6375
        vassert( (brush->prev), "(message) = %s", msg );   // MainFrm.cpp:6376
        vassert( (brush->next), "(message) = %s", msg );   // MainFrm.cpp:6377
        vassert( (brush->prev->next == brush), "(message) = %s", msg );   // MainFrm.cpp:6378
        vassert( (brush->next->prev == brush), "(message) = %s", msg );   // MainFrm.cpp:6379
        vassert( (brush->def), "(message) = %s", msg );   // MainFrm.cpp:6381
        vassert( (brush->owner), "(message) = %s", msg );   // MainFrm.cpp:6382
        vassert( (brush->def->owner == brush->owner->def), "(message) = %s", msg );   // MainFrm.cpp:6383
        vassert( (brush->def->refCount >= 1), "(message) = %s", msg );   // MainFrm.cpp:6384
    }
}
