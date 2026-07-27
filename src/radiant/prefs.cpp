// Radiant preferences, registry persistence, and settings dialog.
#include "stdafx.h"
#include "prefs.h"
#include <stdlib.h>   // atof

// ── the one editor-wide preference instance ──────────────────────────────────
// prefData_t's ctor sets the binary defaults at static-init, so g_PrefsDlg is a
// valid, defaults-loaded object before any view/drag/brush code runs (headless
// included) — which is what lets every reader drop its NULL guard.
static prefData_t s_radiantPrefs;
prefData_t *g_PrefsDlg = &s_radiantPrefs;

prefData_t::prefData_t()
{
    Prefs_SetDefaults( this );
}

// CPrefsDlg::CPrefsDlg (0x44d900) — the constructor's per-field default writes.
void Prefs_SetDefaults( prefData_t *p )
{
    p->m_nMouse_unsure        = 1;            // raw; m_nMouseButtons derived below
    p->m_nMouseButtons        = 3;            // (1!=0)+2
    p->m_nView                = 0;
    p->m_bTextureLock         = 1;
    p->m_bRotateLock          = 1;            // (LoadPrefs default; ctor leaves 0, Load wins)
    p->m_bLightmapLock        = 0;
    p->m_bLoadLast            = 1;
    p->m_bRunBefore           = 0;
    p->camera_mode            = 1;
    p->camera_masked          = 1;
    p->m_bFace                = 1;
    p->m_bRightClick          = 1;
    p->m_bAutoSave            = 1;
    p->m_bNewApplyHandling    = 0;
    p->m_bLoadLastMap         = 0;
    p->m_bTextureWindowSearch = 0;
    p->m_bCleanTinyBrushes    = 0;
    p->m_fTinySize            = 0.5f;
    p->m_nAutoSave            = 5;
    p->m_bSnapShots           = 0;
    p->loose_changes          = 0;
    p->m_nStatusSize          = 10;
    p->m_nMoveSpeed           = 350;
    p->m_nAngleSpeed          = 150;
    p->m_bCamXYUpdate         = 0;
    p->m_bCubicClipping       = 1;
    p->m_nCubicScale          = 13;
    p->m_bALTEdge             = 1;
    p->m_bTextureBar          = 0;
    p->m_bSnapTToGrid         = 0;
    p->linking_keeps_selection= 0;
    p->m_bXZVis               = 0;
    p->m_bYZVis               = 0;
    p->m_bZVis                = 1;
    p->m_bSizePaint           = 1;
    p->b_mCullSky             = 1;
    p->m_dropHeight           = 28;           // ctor default (LoadPrefs key "DropHeight" 28)
    p->m_bForceZeroDropHeight = 0;            // ctor default (no registry key; OnDropSelected gate)
    p->m_bNoClamp             = 0;
    p->m_bDropModel           = 0;
    p->m_bOrientModel         = 0;
    p->m_nRotation            = 45;
    p->farplane               = 8192;         // ctor 0x2000
    p->tolerant_weld          = 24;
    p->vehicle_arrow_time     = 1000;
    p->vehicle_arrow_size     = 128;
    p->splay                  = 128;
    p->m_bChaseMouse          = 1;
    p->m_nEntityShowState     = 65552;        // ctor 65552 (LoadPrefs maps 0→65552)
    p->m_nTextureWindowScale  = 50;
    p->m_bTextureScrollbar    = 1;
    p->m_bSwitchClip          = 1;
    p->m_bSelectWholeEntities = 1;
    p->thick_selection_lines  = 1;
    p->m_bColoredEnts         = 0;
    p->m_bTolerantWeld        = 0;
    p->m_bVertSnapModel       = 0;
    p->m_bVertSnapBrush       = 0;
    p->m_bVertSnapPrefab      = 0;
    p->m_bSelectableModels    = 0;
    p->m_bSelectCurves        = 1;
    p->texture_brush_2d       = 0;
    p->texture_mesh_2d        = 0;
    p->fast_2d_view_dragging  = 1;
    p->detatch_windows        = 0;
    p->transparent_background = 0;
    p->m_nUndoLevels          = 10;
    p->patch_wireframe        = 0;
    p->g_bPatchWeld           = 1;
    p->patch_drill_down       = 1;
    p->entities_off           = 0;
    p->sky_brush_off          = 0;
    p->draw_toggle            = 0;
    p->scale_base             = 100;
    p->scale_range            = 30;
    p->camera_fov             = 65.0f;
    p->camera_use_wheel       = 1;            // ctor dword_25D6064 (LoadPrefs "CameraUseWheel" 1)
    p->model_origin_size      = 4.0f;
    p->prefab_origin_size     = 16.0f;
    p->enable_light_preview   = 1;            // ctor dword_25D6068 = 1
    p->preview_sun_aswell     = 0;            // ctor dword_25D606C = 0
    p->m_strLastProject       = "";
    p->m_strLastMap           = "";
    p->which_game             = "";
    p->ScriptGroupKey         = "script_group";
    p->ScriptGroupTokenKey    = "script_group_tokens";
    p->ScriptColorTeamKey     = "script_color_allies";
    p->ScriptColorKey         = "red";
    p->ScriptSubKey_key       = "script_objective_active";
    p->ScriptSubValue_key     = "";
    p->m_strUserIniPath       = "";
    p->m_strUserFilterPath    = "";
}

// CPrefsDlg::LoadPrefs (0x44e330). Section "Prefs" except RunBefore ("Internals").
void Prefs_LoadPrefs( prefData_t *p )
{
    CWinApp *app = AfxGetApp();
    if ( !app )
        return;

    p->m_nMouse_unsure        = app->GetProfileInt( "Prefs", "MouseButtons", 1 );
    p->m_nMouseButtons        = ( p->m_nMouse_unsure != 0 ) + 2;        // 2 or 3
    p->m_nView                = app->GetProfileInt( "Prefs", "QE4StyleWindows", 0 );
    p->m_bTextureLock         = app->GetProfileInt( "Prefs", "TextureLock", 1 );
    p->m_bRotateLock          = app->GetProfileInt( "Prefs", "RotateLock", 1 );
    p->m_bLightmapLock        = app->GetProfileInt( "Prefs", "LightmapLock", 0 );
    p->m_strLastProject       = app->GetProfileString( "Prefs", "LastProject", "" );
    p->m_strLastMap           = app->GetProfileString( "Prefs", "LastMap", "" );
    p->m_bLoadLast            = app->GetProfileInt( "Prefs", "LoadLast", 1 );
    p->m_bRunBefore           = app->GetProfileInt( "Internals", "RunBefore", 0 );
    p->camera_mode            = app->GetProfileInt( "Prefs", "CameraMode", 1 );
    p->camera_masked          = app->GetProfileInt( "Prefs", "CameraMasked", 1 );
    p->m_bFace                = app->GetProfileInt( "Prefs", "NewFaceGrab", 1 );
    p->m_bRightClick          = app->GetProfileInt( "Prefs", "NewRightClick", 1 );
    p->m_bAutoSave            = app->GetProfileInt( "Prefs", "Autosave", 1 );
    p->m_bNewApplyHandling    = app->GetProfileInt( "Prefs", "ApplyDismissesSurface", 0 );
    p->m_bLoadLastMap         = app->GetProfileInt( "Prefs", "LoadLastMap", 0 );
    p->m_bTextureWindowSearch = app->GetProfileInt( "Prefs", "NewTextureWindowStuff", 0 );
    p->m_bCleanTinyBrushes    = app->GetProfileInt( "Prefs", "CleanTinyBrushes", 0 );
    p->m_fTinySize            = (float)atof( app->GetProfileString( "Prefs", "CleanTinyBrusheSize", "0.5" ) );
    p->m_nAutoSave            = app->GetProfileInt( "Prefs", "AutosaveMinutes", 5 );
    p->m_bSnapShots           = app->GetProfileInt( "Prefs", "Snapshots", 0 );
    p->loose_changes          = app->GetProfileInt( "Prefs", "DefaultSaveNo", 0 );
    p->m_nStatusSize          = app->GetProfileInt( "Prefs", "StatusPointSize", 10 );
    p->m_nMoveSpeed           = app->GetProfileInt( "Prefs", "MoveSpeed", 350 );
    p->m_nAngleSpeed          = app->GetProfileInt( "Prefs", "AngleSpeed", 150 );
    p->m_bCamXYUpdate         = app->GetProfileInt( "Prefs", "CamXYUpdate", 0 );
    p->m_bCubicClipping       = app->GetProfileInt( "Prefs", "CubicClipping", 1 ) != 0;
    p->m_nCubicScale          = app->GetProfileInt( "Prefs", "CubicScale", 13 );
    p->m_bALTEdge             = app->GetProfileInt( "Prefs", "ALTEdgeDrag", 1 );
    p->m_bTextureBar          = app->GetProfileInt( "Prefs", "UseTextureBar", 0 );
    p->which_game             = app->GetProfileString( "Prefs", "WhichGame", "" );
    p->m_bSnapTToGrid         = app->GetProfileInt( "Prefs", "SnapT", 0 );
    p->linking_keeps_selection= app->GetProfileInt( "Prefs", "LinkSelect", 0 );
    p->m_bXZVis               = app->GetProfileInt( "Prefs", "XZVIS", 0 );
    p->m_bYZVis               = app->GetProfileInt( "Prefs", "YZVIS", 0 );
    p->m_bZVis                = app->GetProfileInt( "Prefs", "ZVIS", 1 );
    p->m_bSizePaint           = app->GetProfileInt( "Prefs", "SizePainting", 1 );
    p->b_mCullSky             = app->GetProfileInt( "Prefs", "CullSkies", 1 );
    p->m_dropHeight           = app->GetProfileInt( "Prefs", "DropHeight", 28 );
    p->m_bNoClamp             = app->GetProfileInt( "Prefs", "NoClamp", 0 );
    p->m_bDropModel           = app->GetProfileInt( "Prefs", "DropModel", 0 );
    p->m_bOrientModel         = app->GetProfileInt( "Prefs", "OrientModel", 0 );
    p->ScriptGroupKey         = app->GetProfileString( "Prefs", "ScriptGroupKey", "script_group" );
    p->ScriptGroupTokenKey    = app->GetProfileString( "Prefs", "ScriptGroupTokenKey", "script_group_tokens" );
    p->ScriptColorTeamKey     = app->GetProfileString( "Prefs", "ScriptColorTeamKey", "script_color_allies" );
    p->ScriptColorKey         = app->GetProfileString( "Prefs", "ScriptColorKey", "red" );
    p->ScriptSubKey_key       = app->GetProfileString( "Prefs", "ScriptSubKey_key", "script_objective_active" );
    p->ScriptSubValue_key     = app->GetProfileString( "Prefs", "ScriptSubValue_key", "" );
    p->m_strUserIniPath       = app->GetProfileString( "Prefs", "UserINIPath", "" );
    p->m_strUserFilterPath    = app->GetProfileString( "Prefs", "UserFiltersPath", "" );
    p->m_nRotation            = app->GetProfileInt( "Prefs", "Rotation", 45 );
    p->farplane               = app->GetProfileInt( "Prefs", "Farplane", 8192 );
    p->tolerant_weld          = app->GetProfileInt( "Prefs", "TolerantWeldThreshold", 24 );
    p->vehicle_arrow_time     = app->GetProfileInt( "Prefs", "VehArrowTime", 1000 );
    p->vehicle_arrow_size     = app->GetProfileInt( "Prefs", "VehArrowSize", 128 );
    p->splay                  = app->GetProfileInt( "Prefs", "SplayDistance", 128 );
    p->m_bChaseMouse          = app->GetProfileInt( "Prefs", "ChaseMouse", 1 );
    p->m_nEntityShowState     = app->GetProfileInt( "Prefs", "EntityShow", 0 );
    if ( !p->m_nEntityShowState )
        p->m_nEntityShowState = 65552;
    p->m_nTextureWindowScale  = app->GetProfileInt( "Prefs", "TextureScale", 50 );
    p->m_bTextureScrollbar    = app->GetProfileInt( "Prefs", "TextureScrollbar", 1 );
    p->m_bSwitchClip          = app->GetProfileInt( "Prefs", "SwitchClipKey", 1 );
    p->m_bSelectWholeEntities = app->GetProfileInt( "Prefs", "SelectWholeEntitiesKey", 1 );
    p->thick_selection_lines  = app->GetProfileInt( "Prefs", "ThickLines", 1 );
    p->m_bColoredEnts         = app->GetProfileInt( "Prefs", "ColoredEnts", 0 );
    p->m_bTolerantWeld        = app->GetProfileInt( "Prefs", "TolerantWeld", 0 );
    p->m_bVertSnapModel       = app->GetProfileInt( "Prefs", "VertSnapModel", 0 );
    p->m_bVertSnapBrush       = app->GetProfileInt( "Prefs", "VertSnapBrush", 0 );
    p->m_bVertSnapPrefab      = app->GetProfileInt( "Prefs", "VertSnapPrefab", 0 );
    p->m_bSelectableModels    = app->GetProfileInt( "Prefs", "ModelSelection", 0 );
    p->m_bSelectCurves        = app->GetProfileInt( "Prefs", "SelectCurves", 1 );
    p->texture_brush_2d       = app->GetProfileInt( "Prefs", "2dTextured", 0 );
    p->texture_mesh_2d        = app->GetProfileInt( "Prefs", "2dMeshTextured", 0 );
    p->fast_2d_view_dragging  = app->GetProfileInt( "Prefs", "Fast2dDragging", 1 );
    p->detatch_windows        = app->GetProfileInt( "Prefs", "FloatingWindows", 0 );
    p->transparent_background = app->GetProfileInt( "Prefs", "TransparentBackground", 0 );
    p->m_nUndoLevels          = app->GetProfileInt( "Prefs", "UndoLevels", 10 );
    p->patch_wireframe        = app->GetProfileInt( "Prefs", "PatchWireframe", 0 );
    p->g_bPatchWeld           = app->GetProfileInt( "Prefs", "PatchWeld", 1 ) != 0;
    p->patch_drill_down       = app->GetProfileInt( "Prefs", "PatchDrillDown", 1 ) != 0;
    p->entities_off           = app->GetProfileInt( "Prefs", "EntitiesOff", 0 );
    p->sky_brush_off          = app->GetProfileInt( "Prefs", "SkyBrushOff", 0 );
    p->draw_toggle            = app->GetProfileInt( "Prefs", "DrawToggle", 0 );
    p->scale_base             = app->GetProfileInt( "Prefs", "ScaleBase", 100 );
    p->scale_range            = app->GetProfileInt( "Prefs", "ScaleRange", 30 );
    p->camera_fov             = (float)(unsigned int)app->GetProfileInt( "Prefs", "Fov", 65 );
    p->camera_use_wheel       = app->GetProfileInt( "Prefs", "CameraUseWheel", 1 );
    p->model_origin_size      = (float)(unsigned int)app->GetProfileInt( "Prefs", "ModelOrgSize", 4 );
    p->prefab_origin_size     = (float)(unsigned int)app->GetProfileInt( "Prefs", "PrefabOrgSize", 16 );
    p->enable_light_preview   = app->GetProfileInt( "Prefs", "LightPreviewEnable", 1 );
    p->preview_sun_aswell     = app->GetProfileInt( "Prefs", "SunLightPreviewEnable", 0 );
    // The binary re-reads VertSnap* under the Snap* keys (the later read wins).
    p->m_bVertSnapModel       = app->GetProfileInt( "Prefs", "SnapModel", 0 );
    p->m_bVertSnapBrush       = app->GetProfileInt( "Prefs", "SnapBrush", 0 );
    p->m_bVertSnapPrefab      = app->GetProfileInt( "Prefs", "SnapPrefab", 0 );
    // NOTE: the binary's "if (!RunBefore) SavePrefs()" first-run cascade is omitted
    // here to keep load side-effect-free (the GUI Save-on-OK / toggle handlers create
    // the keys on first change). Behaviour at defaults is identical.
}

// CPrefsDlg::SavePrefs (0x44f280) — clamp the few validated fields, then write all.
void Prefs_SavePrefs( prefData_t *p )
{
    CWinApp *app = AfxGetApp();
    if ( !app )
        return;

    app->WriteProfileInt( "Prefs", "MouseButtons", p->m_nMouse_unsure );
    p->m_nMouseButtons = ( p->m_nMouse_unsure != 0 ) + 2;
    app->WriteProfileInt( "Prefs", "QE4StyleWindows", p->m_nView );
    app->WriteProfileInt( "Prefs", "TextureLock", p->m_bTextureLock );
    app->WriteProfileInt( "Prefs", "RotateLock", p->m_bRotateLock );
    app->WriteProfileInt( "Prefs", "LightmapLock", p->m_bLightmapLock );
    app->WriteProfileInt( "Prefs", "LoadLast", p->m_bLoadLast );
    app->WriteProfileString( "Prefs", "LastProject", p->m_strLastProject );
    app->WriteProfileString( "Prefs", "LastMap", p->m_strLastMap );
    app->WriteProfileInt( "Internals", "RunBefore", p->m_bRunBefore );
    app->WriteProfileInt( "Prefs", "CameraMode", p->camera_mode );
    app->WriteProfileInt( "Prefs", "CameraMasked", p->camera_masked );
    app->WriteProfileInt( "Prefs", "NewFaceGrab", p->m_bFace );
    app->WriteProfileInt( "Prefs", "NewRightClick", p->m_bRightClick );
    app->WriteProfileInt( "Prefs", "Autosave", p->m_bAutoSave );
    app->WriteProfileInt( "Prefs", "LoadLastMap", p->m_bLoadLastMap );
    app->WriteProfileInt( "Prefs", "NewTextureWindowStuff", p->m_bTextureWindowSearch );
    app->WriteProfileInt( "Prefs", "AutosaveMinutes", p->m_nAutoSave );
    app->WriteProfileInt( "Prefs", "Snapshots", p->m_bSnapShots );
    app->WriteProfileInt( "Prefs", "DefaultSaveNo", p->loose_changes );
    app->WriteProfileInt( "Prefs", "StatusPointSize", p->m_nStatusSize );
    app->WriteProfileInt( "Prefs", "CamXYUpdate", p->m_bCamXYUpdate );
    app->WriteProfileInt( "Prefs", "MoveSpeed", p->m_nMoveSpeed );
    app->WriteProfileInt( "Prefs", "AngleSpeed", p->m_nAngleSpeed );
    app->WriteProfileInt( "Prefs", "CubicClipping", p->m_bCubicClipping );
    app->WriteProfileInt( "Prefs", "CubicScale", p->m_nCubicScale );
    app->WriteProfileInt( "Prefs", "ALTEdgeDrag", p->m_bALTEdge );
    app->WriteProfileInt( "Prefs", "UseTextureBar", p->m_bTextureBar );
    app->WriteProfileString( "Prefs", "WhichGame", p->which_game );
    app->WriteProfileInt( "Prefs", "SnapT", p->m_bSnapTToGrid );
    app->WriteProfileInt( "Prefs", "LinkSelect", p->linking_keeps_selection );
    app->WriteProfileInt( "Prefs", "XZVIS", p->m_bXZVis );
    app->WriteProfileInt( "Prefs", "YZVIS", p->m_bYZVis );
    app->WriteProfileInt( "Prefs", "ZVIS", p->m_bZVis );
    app->WriteProfileInt( "Prefs", "SizePainting", p->m_bSizePaint );
    app->WriteProfileInt( "Prefs", "CullSkies", p->b_mCullSky );
    app->WriteProfileInt( "Prefs", "DropHeight", p->m_dropHeight );
    app->WriteProfileInt( "Prefs", "NoClamp", p->m_bNoClamp );
    app->WriteProfileInt( "Prefs", "DropModel", p->m_bDropModel );
    app->WriteProfileInt( "Prefs", "OrientModel", p->m_bOrientModel );
    app->WriteProfileString( "Prefs", "ScriptGroupKey", p->ScriptGroupKey );
    app->WriteProfileString( "Prefs", "ScriptGroupTokenKey", p->ScriptGroupTokenKey );
    app->WriteProfileString( "Prefs", "ScriptColorTeamKey", p->ScriptColorTeamKey );
    app->WriteProfileString( "Prefs", "ScriptColorKey", p->ScriptColorKey );
    app->WriteProfileString( "Prefs", "ScriptSubKey_key", p->ScriptSubKey_key );
    app->WriteProfileString( "Prefs", "ScriptSubValue_key", p->ScriptSubValue_key );
    app->WriteProfileString( "Prefs", "UserINIPath", p->m_strUserIniPath );
    app->WriteProfileString( "Prefs", "UserFiltersPath", p->m_strUserFilterPath );
    app->WriteProfileInt( "Prefs", "Rotation", p->m_nRotation );
    app->WriteProfileInt( "Prefs", "Farplane", p->farplane );
    app->WriteProfileInt( "Prefs", "TolerantWeldThreshold", p->tolerant_weld );
    app->WriteProfileInt( "Prefs", "VehArrowTime", p->vehicle_arrow_time );
    app->WriteProfileInt( "Prefs", "VehArrowSize", p->vehicle_arrow_size );
    app->WriteProfileInt( "Prefs", "SplayDistance", p->splay );
    app->WriteProfileInt( "Prefs", "ModelSelection", p->m_bSelectableModels );
    // IDA SavePrefs 0x44f280 does NOT write "SelectCurves" here (LoadPrefs reads it, but the
    // binary never persists it — a latent quirk); matched by omitting the write.  The binary
    // instead re-writes "ModelSelection" a SECOND time later (after VertSnapPrefab — restored
    // below), so this was a swapped/invented write.
    app->WriteProfileInt( "Prefs", "ChaseMouse", p->m_bChaseMouse );
    app->WriteProfileInt( "Prefs", "EntityShow", p->m_nEntityShowState );
    app->WriteProfileInt( "Prefs", "TextureScale", p->m_nTextureWindowScale );
    app->WriteProfileInt( "Prefs", "TextureScrollbar", p->m_bTextureScrollbar );
    app->WriteProfileInt( "Prefs", "SwitchClipKey", p->m_bSwitchClip );
    app->WriteProfileInt( "Prefs", "SelectWholeEntitiesKey", p->m_bSelectWholeEntities );
    app->WriteProfileInt( "Prefs", "ThickLines", p->thick_selection_lines );
    app->WriteProfileInt( "Prefs", "ColoredEnts", p->m_bColoredEnts );
    app->WriteProfileInt( "Prefs", "TolerantWeld", p->m_bTolerantWeld );
    app->WriteProfileInt( "Prefs", "VertSnapModel", p->m_bVertSnapModel );
    app->WriteProfileInt( "Prefs", "VertSnapBrush", p->m_bVertSnapBrush );
    app->WriteProfileInt( "Prefs", "VertSnapPrefab", p->m_bVertSnapPrefab );
    app->WriteProfileInt( "Prefs", "ModelSelection", p->m_bSelectableModels );  // IDA v78: binary writes ModelSelection a 2nd time here
    app->WriteProfileInt( "Prefs", "2dTextured", p->texture_brush_2d );
    app->WriteProfileInt( "Prefs", "2dMeshTextured", p->texture_mesh_2d );
    app->WriteProfileInt( "Prefs", "Fast2dDragging", p->fast_2d_view_dragging );
    app->WriteProfileInt( "Prefs", "FloatingWindows", p->detatch_windows );
    app->WriteProfileInt( "Prefs", "TransparentBackground", p->transparent_background );
    app->WriteProfileInt( "Prefs", "UndoLevels", p->m_nUndoLevels );
    app->WriteProfileInt( "Prefs", "PatchWireframe", p->patch_wireframe );
    app->WriteProfileInt( "Prefs", "PatchWeld", p->g_bPatchWeld );
    app->WriteProfileInt( "Prefs", "PatchDrillDown", p->patch_drill_down );
    app->WriteProfileInt( "Prefs", "DrawToggle", p->draw_toggle );
    app->WriteProfileInt( "Prefs", "EntitiesOff", p->entities_off );
    app->WriteProfileInt( "Prefs", "SkyBrushOff", p->sky_brush_off );
    app->WriteProfileInt( "Prefs", "CameraUseWheel", p->camera_use_wheel );
    app->WriteProfileInt( "Prefs", "ModelOrgSize", (int)p->model_origin_size );
    app->WriteProfileInt( "Prefs", "PrefabOrgSize", (int)p->prefab_origin_size );
    app->WriteProfileInt( "Prefs", "LightPreviewEnable", p->enable_light_preview );
    app->WriteProfileInt( "Prefs", "SunLightPreviewEnable", p->preview_sun_aswell );
    app->WriteProfileInt( "Prefs", "SnapModel", p->m_bVertSnapModel );
    app->WriteProfileInt( "Prefs", "SnapBrush", p->m_bVertSnapBrush );
    app->WriteProfileInt( "Prefs", "SnapPrefab", p->m_bVertSnapPrefab );
    if ( p->scale_base <= 1 )
        p->scale_base = 100;
    app->WriteProfileInt( "Prefs", "ScaleBase", p->scale_base );
    if ( p->scale_base - p->scale_range <= 0 )
        p->scale_range = p->scale_base - 1;
    app->WriteProfileInt( "Prefs", "ScaleRange", p->scale_range );
    if ( p->camera_fov < 2.0f )          p->camera_fov = 2.0f;
    else if ( p->camera_fov > 160.0f )   p->camera_fov = 160.0f;
    app->WriteProfileInt( "Prefs", "Fov", (int)p->camera_fov );
}

void Prefs_Init( bool loadFromRegistry )
{
    // s_radiantPrefs is already defaults-constructed at static init; re-assert so a
    // re-init (or a future non-static instance) is well-defined.
    Prefs_SetDefaults( g_PrefsDlg );
    if ( loadFromRegistry )
        Prefs_LoadPrefs( g_PrefsDlg );
}

// ─────────────────────────────────────────────────────────────────────────────
//  CPrefsDlg — the GUI editor for prefData_t (Edit→Preferences).
//
//  FULL FAITHFUL PORT (2026-07-03): uses the real dialog TEMPLATE 127
//  (IDD_COD4RADIANT_PREFERENCES, extracted from IW3xRadiant.exe into radiant.rc, font
//  normalized) and binds EVERY control the binary's CPrefsDlg::DoDataExchange (0x44de40)
//  binds, mapping each control id → its prefData_t field (field↔registry-key map from
//  CPrefsDlg::LoadPrefs 0x44e330 / SavePrefs 0x44f280).  A few controls drive parked
//  features but are still BOUND (persist their field) per the operator directive.
//  The 4-checkbox IDD_RADIANT_PREFS_MINI stand-in is retired.
// ─────────────────────────────────────────────────────────────────────────────
class CPrefsDlg : public CDialog
{
public:
    CPrefsDlg( CWnd *parent ) : CDialog( IDD_COD4RADIANT_PREFERENCES, parent ) {}

protected:
    // Radio group indices (DDX_Radio): 0-based position of the checked button.
    int  m_rMouse;      // 1003/1005  "2 button"/"3 button"  → m_nMouseButtons (2→0, 3→1)
    int  m_rView;       // 1006..     view-mode radio         → m_nView
    // Checkboxes (BOOL) — control id ↔ prefData_t field (via 0x44de40 DDX + text/LoadPrefs).
    BOOL m_bLoadLast, m_bFace, m_bRightClick, m_bAutoSave, m_bLoadLastMap, m_bTexSubset;
    BOOL m_bSnapshots, m_bLoseChanges, m_bCamXYUpdate, m_bUseWheel, m_bAltAlwaysMove;
    BOOL m_bSnapTGrid, m_bLinkKeepSel, m_bPaintSizing, m_bCullSky, m_bDontClamp;
    BOOL m_bTexToolbar;   // IDC 1047 "Texture toolbar" ↔ m_bTextureBar (UseTextureBar)
    BOOL m_bChaseMouse, m_bTexScrollbar, m_bThickLines, m_bColoredEnts, m_bTexBrush2d;
    BOOL m_bTexMesh2d, m_bFast2dDrag, m_bDetachWin, m_bTransBg;
    // Edit fields (ints / floats).
    int   m_nAutoSaveMin, m_nStatusSize, m_nRotation, m_nFarplane, m_nUndoLevels;
    int   m_nTolerantWeld, m_nSplay, m_nDropHeight, m_nScaleBase, m_nScaleRange;
    int   m_nVehArrowTime, m_nVehArrowSize;
    float m_fFov, m_fModelOrg, m_fPrefabOrg;
    CString m_sUserIni, m_sUserFilters;

    virtual void DoDataExchange( CDataExchange *pDX )
    {
        CDialog::DoDataExchange( pDX );
        // 0x44de9d/af: radios.
        DDX_Radio( pDX, 1003, m_rMouse );          // 2/3 button
        DDX_Radio( pDX, 1006, m_rView );           // view mode
        // Checkboxes (id → field).
        DDX_Check( pDX, 1021, m_bLoadLast );       // Load last project on open   (LoadLast)
        DDX_Check( pDX, 1040, m_bFace );           // Face selection              (NewFaceGrab)
        DDX_Check( pDX, 1042, m_bRightClick );     // Right click to drop entities(NewRightClick)
        DDX_Check( pDX, 1023, m_bAutoSave );       // Auto save every             (Autosave)
        DDX_Text ( pDX, 1065, m_nAutoSaveMin );    // autosave minutes            (AutosaveMinutes)
        DDX_Check( pDX, 1024, m_bLoadLastMap );    // Load last map on open       (LoadLastMap)
        DDX_Check( pDX, 1045, m_bTexSubset );      // Texture subset              (NewTextureWindowStuff)
        DDX_Check( pDX, 1094, m_bSnapshots );      // Snapshots                   (Snapshots)
        DDX_Check( pDX, 1095, m_bLoseChanges );    // Lose changes?               (DefaultSaveNo)
        DDX_Text ( pDX, 1201, m_nStatusSize );     // Status point size           (StatusPointSize)
        DDX_Check( pDX, 1223, m_bCamXYUpdate );    // Update XY on drag           (CamXYUpdate)
        DDX_Check( pDX, 1538, m_bUseWheel );       // Use mouse wheel in camera   (CameraUseWheel)
        DDX_Check( pDX, 1246, m_bAltAlwaysMove );  // ALT always move             (ALTEdgeDrag)
        DDX_Check( pDX, 1047, m_bTexToolbar );     // Texture toolbar             (UseTextureBar) [0x44de40 this+143]
        DDX_Check( pDX, 1051, m_bSnapTGrid );      // Snap T to Grid              (SnapT)
        DDX_Check( pDX, 1085, m_bLinkKeepSel );    // Linking keeps selection     (LinkSelect)
        DDX_Check( pDX, 1084, m_bPaintSizing );    // Paint sizing info           (SizePainting)
        DDX_Check( pDX, 1517, m_bCullSky );        // Cull sky on clip            (CullSkies)
        DDX_Check( pDX, 1255, m_bDontClamp );      // Don't clamp plane points    (NoClamp)
        DDX_Text ( pDX, 1026, m_sUserIni );        // User INI path               (UserINIPath)
        DDX_Text ( pDX, 1685, m_sUserFilters );    // User Filters path           (UserFiltersPath)
        DDX_Text ( pDX, 1204, m_nRotation );       // Rotation inc                (Rotation)
        DDX_Text ( pDX, 1027, m_nFarplane );       // Farplane                    (Farplane)
        DDX_Text ( pDX, 1456, m_nTolerantWeld );   // Tolerant Weld               (TolerantWeldThreshold)
        DDX_Text ( pDX, 1480, m_nVehArrowTime );   // Vehicle Arrow Time          (VehArrowTime)
        DDX_Text ( pDX, 1481, m_nVehArrowSize );   // Vehicle Arrow Size          (VehArrowSize)
        DDX_Text ( pDX, 1457, m_nSplay );          // Splay Distance              (SplayDistance)
        DDX_Text ( pDX, 1459, m_nDropHeight );     // Drop Height                 (DropHeight)
        DDX_Check( pDX, 1249, m_bChaseMouse );     // Mouse chaser                (ChaseMouse)
        DDX_Check( pDX, 1054, m_bTexScrollbar );   // Texture scrollbar           (TextureScrollbar)
        DDX_Check( pDX, 1486, m_bThickLines );     // Thick selection lines       (ThickLines)
        DDX_Check( pDX, 1420, m_bColoredEnts );    // Ents use '_color' value     (ColoredEnts)
        DDX_Check( pDX, 1423, m_bTexBrush2d );     // Texture brushes in 2d       (2dTextured)
        DDX_Check( pDX, 1424, m_bTexMesh2d );      // Texture meshes in 2d        (2dMeshTextured)
        DDX_Check( pDX, 1672, m_bFast2dDrag );     // Fast 2d view dragging       (Fast2dDragging)
        DDX_Check( pDX, 1674, m_bDetachWin );      // Detached Windows            (FloatingWindows)
        DDX_Check( pDX, 1687, m_bTransBg );        // Transparent background      (TransparentBackground)
        DDX_Text ( pDX, 1208, m_nUndoLevels );     // Undo Levels                 (UndoLevels)
        DDX_Text ( pDX, 1461, m_nScaleBase );      // Scale Base                  (ScaleBase)
        DDX_Text ( pDX, 1463, m_nScaleRange );     // Scale Range                 (ScaleRange)
        DDX_Text ( pDX, 1700, m_fFov );            // FOV                         (Fov)
        DDX_Text ( pDX, 1559, m_fModelOrg );       // Model Origin Size           (ModelOrgSize)
        DDX_Text ( pDX, 1560, m_fPrefabOrg );      // Prefab Origin Size          (PrefabOrgSize)
    }

    virtual BOOL OnInitDialog()
    {
        CDialog::OnInitDialog();
        prefData_t *p = g_PrefsDlg;
        m_rMouse         = ( p->m_nMouseButtons == 3 ) ? 1 : 0;
        m_rView          = p->m_nView;
        m_bLoadLast      = p->m_bLoadLast != 0;
        m_bFace          = p->m_bFace != 0;
        m_bRightClick    = p->m_bRightClick != 0;
        m_bAutoSave      = p->m_bAutoSave != 0;
        m_nAutoSaveMin   = p->m_nAutoSave;
        m_bLoadLastMap   = p->m_bLoadLastMap != 0;
        m_bTexSubset     = p->m_bTextureWindowSearch != 0;
        m_bSnapshots     = p->m_bSnapShots != 0;
        m_bLoseChanges   = p->loose_changes != 0;
        m_nStatusSize    = p->m_nStatusSize;
        m_bCamXYUpdate   = p->m_bCamXYUpdate != 0;
        m_bUseWheel      = p->camera_use_wheel != 0;
        m_bAltAlwaysMove = p->m_bALTEdge != 0;
        m_bTexToolbar    = p->m_bTextureBar != 0;
        m_bSnapTGrid     = p->m_bSnapTToGrid != 0;
        m_bLinkKeepSel   = p->linking_keeps_selection != 0;
        m_bPaintSizing   = p->m_bSizePaint != 0;
        m_bCullSky       = p->b_mCullSky != 0;
        m_bDontClamp     = p->m_bNoClamp != 0;
        m_sUserIni       = p->m_strUserIniPath;
        m_sUserFilters   = p->m_strUserFilterPath;
        m_nRotation      = p->m_nRotation;
        m_nFarplane      = p->farplane;
        m_nTolerantWeld  = p->tolerant_weld;
        m_nVehArrowTime  = p->vehicle_arrow_time;
        m_nVehArrowSize  = p->vehicle_arrow_size;
        m_nSplay         = p->splay;
        m_nDropHeight    = p->m_dropHeight;
        m_bChaseMouse    = p->m_bChaseMouse != 0;
        m_bTexScrollbar  = p->m_bTextureScrollbar != 0;
        m_bThickLines    = p->thick_selection_lines != 0;
        m_bColoredEnts   = p->m_bColoredEnts != 0;
        m_bTexBrush2d    = p->texture_brush_2d != 0;
        m_bTexMesh2d     = p->texture_mesh_2d != 0;
        m_bFast2dDrag    = p->fast_2d_view_dragging != 0;
        m_bDetachWin     = p->detatch_windows != 0;
        m_bTransBg       = p->transparent_background != 0;
        m_nUndoLevels    = p->m_nUndoLevels;
        m_nScaleBase     = p->scale_base;
        m_nScaleRange    = p->scale_range;
        m_fFov           = p->camera_fov;
        m_fModelOrg      = p->model_origin_size;
        m_fPrefabOrg     = p->prefab_origin_size;
        UpdateData( FALSE );   // push settings → controls
        return TRUE;
    }

    virtual void OnOK()
    {
        UpdateData( TRUE );    // pull controls → members
        prefData_t *p = g_PrefsDlg;
        p->m_nMouse_unsure    = m_rMouse;                 // raw registry value (0/1)
        p->m_nMouseButtons    = m_rMouse ? 3 : 2;
        p->m_nView            = m_rView;
        p->m_bLoadLast        = m_bLoadLast ? 1 : 0;
        p->m_bFace            = m_bFace ? 1 : 0;
        p->m_bRightClick      = m_bRightClick ? 1 : 0;
        p->m_bAutoSave        = m_bAutoSave ? 1 : 0;
        p->m_nAutoSave        = m_nAutoSaveMin;
        p->m_bLoadLastMap     = m_bLoadLastMap ? 1 : 0;
        p->m_bTextureWindowSearch = m_bTexSubset ? 1 : 0;
        p->m_bSnapShots       = m_bSnapshots ? 1 : 0;
        p->loose_changes      = m_bLoseChanges ? 1 : 0;
        p->m_nStatusSize      = m_nStatusSize;
        p->m_bCamXYUpdate     = m_bCamXYUpdate ? 1 : 0;
        p->camera_use_wheel   = m_bUseWheel ? 1 : 0;
        p->m_bALTEdge         = m_bAltAlwaysMove ? 1 : 0;
        p->m_bTextureBar      = m_bTexToolbar ? 1 : 0;
        p->m_bSnapTToGrid     = m_bSnapTGrid ? 1 : 0;
        p->linking_keeps_selection = m_bLinkKeepSel ? 1 : 0;
        p->m_bSizePaint       = m_bPaintSizing ? 1 : 0;
        p->b_mCullSky         = m_bCullSky ? 1 : 0;
        p->m_bNoClamp         = m_bDontClamp ? 1 : 0;
        p->m_strUserIniPath   = m_sUserIni;
        p->m_strUserFilterPath= m_sUserFilters;
        p->m_nRotation        = m_nRotation;
        p->farplane           = m_nFarplane;
        p->tolerant_weld      = m_nTolerantWeld;
        p->vehicle_arrow_time = m_nVehArrowTime;
        p->vehicle_arrow_size = m_nVehArrowSize;
        p->splay              = m_nSplay;
        p->m_dropHeight       = m_nDropHeight;
        p->m_bChaseMouse      = m_bChaseMouse ? 1 : 0;
        p->m_bTextureScrollbar= m_bTexScrollbar ? 1 : 0;
        p->thick_selection_lines = m_bThickLines ? 1 : 0;
        p->m_bColoredEnts     = m_bColoredEnts ? 1 : 0;
        p->texture_brush_2d   = m_bTexBrush2d ? 1 : 0;
        p->texture_mesh_2d    = m_bTexMesh2d ? 1 : 0;
        p->fast_2d_view_dragging = m_bFast2dDrag ? 1 : 0;
        p->detatch_windows    = m_bDetachWin ? 1 : 0;
        p->transparent_background = m_bTransBg ? 1 : 0;
        p->m_nUndoLevels      = m_nUndoLevels;
        p->scale_base         = m_nScaleBase;
        p->scale_range        = m_nScaleRange;
        p->camera_fov         = m_fFov;
        p->model_origin_size  = m_fModelOrg;
        p->prefab_origin_size = m_fPrefabOrg;
        Prefs_SavePrefs( g_PrefsDlg );
        CDialog::OnOK();
    }

    // ── The binary's CPrefsDlg message map (entries @0x6E1900) — 6 entries, ALL of
    //    which were missing from the port (the map was empty), so the two "..." browse
    //    buttons and the view-mode radio group were inert.  Ported verbatim.
    //      1029 → OnBtnBrowseuserini     0x44FE80
    //      1673 → OnBtnBrowseuserfilter  0x44FF90
    //      1006 / 1009 / 1014 → SetGamePrefs 0x4500A0   (the 3 view-mode radios)
    //      1095 → nullsub_117            0x450110       (genuinely empty in the binary)
    afx_msg void OnBtnBrowseuserini()      // 0x44FE80
    {
        UpdateData( TRUE );
        CFileDialog dlg( TRUE, nullptr, nullptr,
                         OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,   // idb dwFlags 6
                         "INI files (*.ini)|*.ini||", this );
        if ( dlg.DoModal() == IDOK )
        {
            m_sUserIni = dlg.GetPathName();
            UpdateData( FALSE );
        }
    }

    afx_msg void OnBtnBrowseuserfilter()   // 0x44FF90
    {
        UpdateData( TRUE );
        CFileDialog dlg( TRUE, nullptr, nullptr,
                         OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,   // idb dwFlags 6
                         "TXT files (*.txt)|*.txt||", this );
        if ( dlg.DoModal() == IDOK )
        {
            m_sUserFilters = dlg.GetPathName();
            UpdateData( FALSE );
        }
    }

    // 0x4500A0 — the view-mode radio group: only the QE4-style layout (index 1) supports
    // the detached-windows / transparent-background options, so those two checkboxes are
    // enabled iff m_nView == 1.  (m_rView IS the binary's m_nView — same DDX_Radio 1006.)
    afx_msg void OnSetGamePrefs()          // 0x4500A0
    {
        UpdateData( TRUE );
        const BOOL enable = ( m_rView == 1 );
        GetDlgItem( 1674 )->EnableWindow( enable );   // Detached Windows
        GetDlgItem( 1687 )->EnableWindow( enable );   // Transparent background
    }

    // 0x450110 — nullsub_117.  Genuinely EMPTY in the binary (the "Lose changes?"
    // checkbox 1095 is bound by DDX only); wired so the map matches entry-for-entry.
    afx_msg void OnPrefsNullsub1095() {}

    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP( CPrefsDlg, CDialog )
    ON_BN_CLICKED( 1029, OnBtnBrowseuserini )     // "..." browse User INI      (0x44FE80)
    ON_BN_CLICKED( 1673, OnBtnBrowseuserfilter )  // "..." browse User Filters  (0x44FF90)
    ON_BN_CLICKED( 1006, OnSetGamePrefs )         // view-mode radio 0          (0x4500A0)
    ON_BN_CLICKED( 1009, OnSetGamePrefs )         // view-mode radio 1          (0x4500A0)
    ON_BN_CLICKED( 1014, OnSetGamePrefs )         // view-mode radio 2          (0x4500A0)
    ON_BN_CLICKED( 1095, OnPrefsNullsub1095 )     // "Lose changes?"            (0x450110 nullsub)
END_MESSAGE_MAP()

int Prefs_ShowDialog( CWnd *parent )
{
    // Refresh from the registry first (mirrors OnPrefs 0x426950 → LoadPrefs), so the
    // dialog reflects what is actually persisted.
    Prefs_LoadPrefs( g_PrefsDlg );
    CPrefsDlg dlg( parent );
    INT_PTR r = dlg.DoModal();
    return ( r == IDOK ) ? IDOK : IDCANCEL;
}
