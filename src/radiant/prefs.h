// Editor-wide preference data and the MFC settings dialog.
#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif

// prefData_t — the editor preference set. Field names + default values are the
// binary's CPrefsDlg members (LoadPrefs/SavePrefs registry keys in the comments).
struct prefData_t
{
    // ── mouse / windowing ──────────────────────────────────────────────────────
    int   m_nMouse_unsure;        // "MouseButtons"            raw registry value (1)
    int   m_nMouseButtons;        //  derived (m_nMouse_unsure!=0)+2 → 2 or 3 (3)
    int   m_nView;                // "QE4StyleWindows"         (0)
    // ── locks ──────────────────────────────────────────────────────────────────
    int   m_bTextureLock;         // "TextureLock"             (1)
    int   m_bRotateLock;          // "RotateLock"              (1)
    int   m_bLightmapLock;        // "LightmapLock"            (0)
    // ── load / save behaviour ───────────────────────────────────────────────────
    int   m_bLoadLast;            // "LoadLast"                (1)
    int   m_bRunBefore;           // "Internals\RunBefore"     (0) — first-run sentinel
    int   camera_mode;            // "CameraMode"              (1)
    int   camera_masked;          // "CameraMasked"            (1)
    int   m_bFace;                // "NewFaceGrab"             (1)
    int   m_bRightClick;          // "NewRightClick"           (1)
    int   m_bAutoSave;            // "Autosave"                (1)
    int   m_bNewApplyHandling;    // "ApplyDismissesSurface"   (0)
    int   m_bLoadLastMap;         // "LoadLastMap"             (0)
    int   m_bTextureWindowSearch; // "NewTextureWindowStuff"   (0)
    int   m_bCleanTinyBrushes;    // "CleanTinyBrushes"        (0)
    float m_fTinySize;            // "CleanTinyBrusheSize"     (0.5)
    int   m_nAutoSave;            // "AutosaveMinutes"         (5)
    int   m_bSnapShots;           // "Snapshots"               (0)
    int   loose_changes;          // "DefaultSaveNo"           (0)
    int   m_nStatusSize;          // "StatusPointSize"         (10)
    int   m_nMoveSpeed;           // "MoveSpeed"               (350)
    int   m_nAngleSpeed;          // "AngleSpeed"              (150)
    int   m_bCamXYUpdate;         // "CamXYUpdate"             (0)
    int   m_bCubicClipping;       // "CubicClipping"           (1)
    int   m_nCubicScale;          // "CubicScale"              (13)
    int   m_bALTEdge;             // "ALTEdgeDrag"             (1)
    int   m_bTextureBar;          // "UseTextureBar"           (0)
    int   m_bSnapTToGrid;         // "SnapT"                   (0)
    int   linking_keeps_selection;// "LinkSelect"              (0)
    int   m_bXZVis;               // "XZVIS"                   (0)
    int   m_bYZVis;               // "YZVIS"                   (0)
    int   m_bZVis;                // "ZVIS"                    (1)
    int   m_bSizePaint;           // "SizePainting"            (1)
    int   b_mCullSky;             // "CullSkies"               (1)
    int   m_dropHeight;           // "DropHeight"              (28)
    int   m_bForceZeroDropHeight; // (no registry key; runtime) (0) — when set, the drop ignores
                                  //   m_dropHeight (CPrefsDlg @+0x2D4; not in Load/SavePrefs)
    int   m_bNoClamp;             // "NoClamp"                 (0) — grid snap when 0
    int   m_bDropModel;           // "DropModel"               (0)
    int   m_bOrientModel;         // "OrientModel"             (0)
    int   m_nRotation;            // "Rotation"                (45)
    int   farplane;               // "Farplane"                (8192)
    int   tolerant_weld;          // "TolerantWeldThreshold"   (24)
    int   vehicle_arrow_time;     // "VehArrowTime"            (1000)
    int   vehicle_arrow_size;     // "VehArrowSize"            (128)
    int   splay;                  // "SplayDistance"           (128)
    int   m_bChaseMouse;          // "ChaseMouse"              (1)
    int   m_nEntityShowState;     // "EntityShow"              (0→65552 default)
    int   m_nTextureWindowScale;  // "TextureScale"            (50)
    int   m_bTextureScrollbar;    // "TextureScrollbar"        (1)
    int   m_bSwitchClip;          // "SwitchClipKey"           (1)
    int   m_bSelectWholeEntities; // "SelectWholeEntitiesKey"  (1)
    int   thick_selection_lines;  // "ThickLines"              (1)
    int   m_bColoredEnts;         // "ColoredEnts"             (0)
    int   m_bTolerantWeld;        // "TolerantWeld"            (0)
    int   m_bVertSnapModel;       // "VertSnapModel"/"SnapModel"   (0)
    int   m_bVertSnapBrush;       // "VertSnapBrush"/"SnapBrush"   (0)
    int   m_bVertSnapPrefab;      // "VertSnapPrefab"/"SnapPrefab" (0)
    int   m_bSelectableModels;    // "ModelSelection"          (0)
    int   texture_brush_2d;       // "2dTextured"              (0)
    int   texture_mesh_2d;        // "2dMeshTextured"          (0)
    int   fast_2d_view_dragging;  // "Fast2dDragging"          (1)
    int   detatch_windows;        // "FloatingWindows"         (0)
    int   transparent_background; // "TransparentBackground"   (0)
    int   m_nUndoLevels;          // "UndoLevels"              (10)
    int   patch_wireframe;        // "PatchWireframe"          (0)
    int   g_bPatchWeld;           // "PatchWeld"               (1)
    int   patch_drill_down;       // "PatchDrillDown"          (1)
    int   m_bSelectCurves;        // "SelectCurves"            (1) — when 0 the Selection
                                  //   ray skips patch brushes ("Don't select curves" toolbar toggle, 32852)
    int   entities_off;           // "EntitiesOff"             (0)
    int   sky_brush_off;          // "SkyBrushOff"             (0)
    int   draw_toggle;            // "DrawToggle"              (0)
    int   scale_base;             // "ScaleBase"               (100)
    int   scale_range;            // "ScaleRange"              (30)
    float camera_fov;             // "Fov"                     (65)
    int   camera_use_wheel;       // "CameraUseWheel"          (1)
    float model_origin_size;      // "ModelOrgSize"            (4)
    float prefab_origin_size;     // "PrefabOrgSize"           (16)
    int   enable_light_preview;   // "LightPreviewEnable"      (1) — light/sun preview gate
    int   preview_sun_aswell;     // "SunLightPreviewEnable"   (0) — sun shading in preview
    // ── string settings (CString; not consumed by the wired band-aids yet, but
    //    persisted faithfully so the round-trip is complete) ──────────────────────
    CString m_strLastProject;     // "LastProject"
    CString m_strLastMap;         // "LastMap"
    CString which_game;           // "WhichGame"
    CString ScriptGroupKey;       // "ScriptGroupKey"          ("script_group")
    CString ScriptGroupTokenKey;  // "ScriptGroupTokenKey"     ("script_group_tokens")
    CString ScriptColorTeamKey;   // "ScriptColorTeamKey"      ("script_color_allies")
    CString ScriptColorKey;       // "ScriptColorKey"          ("red")
    CString ScriptSubKey_key;     // "ScriptSubKey_key"        ("script_objective_active")
    CString ScriptSubValue_key;   // "ScriptSubValue_key"
    CString m_strUserIniPath;     // "UserINIPath"
    CString m_strUserFilterPath;  // "UserFiltersPath"

    prefData_t();                 // sets the binary's defaults (Prefs_SetDefaults)
};

// The single editor-wide preference instance (the binary's g_PrefsDlg @0x73c704).
// Points at a static prefData_t, non-NULL from static-init onward — so the view/
// drag/brush readers never need a NULL guard.
extern prefData_t *g_PrefsDlg;

// Set the binary's CPrefsDlg::CPrefsDlg defaults into *p (also the ctor body).
void Prefs_SetDefaults( prefData_t *p );
// Load every setting from the registry (CPrefsDlg::LoadPrefs 0x44e330).
void Prefs_LoadPrefs( prefData_t *p );
// Write every setting to the registry (CPrefsDlg::SavePrefs 0x44f280).
void Prefs_SavePrefs( prefData_t *p );
// Startup: ensure defaults; in GUI mode also overlay the registry values. Headless
// gate runs pass loadFromRegistry=false for determinism (defaults only).
void Prefs_Init( bool loadFromRegistry );

// GUI: open the preferences dialog (Edit→Preferences). Returns IDOK if the user
// applied (and the new settings were saved), IDCANCEL otherwise. parent may be NULL.
int  Prefs_ShowDialog( CWnd *parent );
