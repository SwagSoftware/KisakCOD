#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// CMainFrame (387 methods in the binary) + the editor's window/dialog classes.
// Handler EAs come from the IDB message-map dump; bodies live in mainfrm.cpp.

#include "stdafx.h"

// ─── CCamWnd — the 3D perspective camera view ───────────────────────────
// camera_s — IDB camera_s (0x7c). The basis vectors (vpn/vright/vup) are rebuilt
// each frame from angles by Cam_BuildMatrix; forward/right are the yaw-plane
// movement basis used by the WASD/arrow fly controls.
struct camera_s   // 0x7c
{
    int   width;          // 0x00  client width  (px)
    int   height;         // 0x04  client height (px)
    bool  timing;         // 0x08
    char  pad_timing[3];  // 0x09
    float origin[3];      // 0x0c  camera world position
    float angles[3];      // 0x18  pitch/yaw/roll
    int   draw_mode;      // 0x24  0=wire 1=fullbright 2=normal 3=view 4=textures
    float color[3];       // 0x28
    float forward[3];     // 0x34  yaw-plane forward (movement)
    float right[3];       // 0x40  yaw-plane right   (movement)
    float up[3];          // 0x4c
    float vup[3];         // 0x58  view up
    float vpn[3];         // 0x64  view forward (normal)
    float vright[3];      // 0x70  view right
};
static_assert(sizeof(camera_s) == 0x7c, "camera_s must be 0x7c (IDB)");

class CCamWnd : public CWnd
{
public:
    CCamWnd();
    camera_s camera;                  // embedded camera state
    int    m_nWidth  = 0;
    int    m_nHeight = 0;
#if 0 // ── DISABLED: prior-session FPS mouse-look camera (restore-on-demand) ──
    // KEPT per operator request; the binary-faithful RMB-joystick / camera_mode scheme
    // supersedes it.  To re-enable: flip this #if 0 to #if 1 and disable the faithful
    // OnRButtonDown/Up/OnMouseMove bodies in camwnd.cpp (matching DISABLED markers).
    bool   m_bLooking = false;        // RMB held → mouse-look
    CPoint m_ptLook = { 0, 0 };       // last cursor pos during look
#endif

    // 3D-view mouse interaction state (CamWnd_DropModelsToPlane / Cam_MouseControl /
    // Cam_MouseUp / Cam_MouseMoved — IDB 0x403d30 / 0x403950 / 0x404f70 / 0x404fc0).
    // Offsets in trailing comments are the CCamWnd IDB field offsets (this != binary
    // layout — CWnd base differs — they are recorded only to map each member to the IDB).
    unsigned int m_nCambuttonstate = 0;   // +0xd4  MK_ flags of the active press
    CPoint m_ptButton  = { 0, 0 };        // +0xd8  press point (flipped Y, client coords)
    bool   cam_was_not_dragged = false;   // +0xe0  set on a plain RMB press (no drag yet)
    CPoint m_ptCursor  = { 0, 0 };        // +0xe4  cursor anchor for the view-control drags
    CPoint m_ptCursor3 = { 0, 0 };        // (port) GetCursorPos at press — CamWnd_DropModelsToPlane
    CPoint m_ptLastCursor = { 0, 0 };     // +0xec  OnMouseMove dedup (skip no-move events)
    int    prob_some_cursor = 0;          // +0x128 accumulated cursor dx (texture rotate/shift snap)
    int    x47 = 0;                       // +0x12c accumulated cursor dy (texture rotate/shift snap)
    int    cursor_visible = 1;            // +0x130 cursor-shown flag; Cam_MouseUp restores it to 1

    // ── light-region preview records (Regions_ForSelected 0x406F10 reads these) ──
    // Each record (56 B = IDB stride 14 dwords): { int instOrBrush; int arg2;
    // orientation_t orient(0x30) }.  Filled by CCamWnd_AddLightPreview (0x406200) from
    // the Light-Preview menu commands.  +0x134 / +0x2F4.
    struct LightPreviewRec { int inst; int arg2; orientation_t orient; };
    LightPreviewRec light_preview_arr[16] = {};   // +0x134
    int             light_preview_count = 0;      // +0x2F4

    // ── Cubic/frustum clip-plane state (CullCubic 0x4056d0 / Cam_Fov 0x405460 /
    //    Cam_SetupClipPlanes sub_405620 0x405620) ────────────────────────────────
    // NOTE: this port's CCamWnd is NOT byte-compatible with the binary (the CWnd base
    // differs), so these are named members, not raw offsets.  The trailing IDB field
    // names map each to the binary layout.  Cam_Fov computes the WORLD-space side-plane
    // normals + axis-index triples (clipWorldNormal[2]/clipWorldAxis[2], = IDB x33..x44)
    // from the camera basis and FOV, then Cam_SetupClipPlanes transforms them into a
    // given orientation's local space (clipBoxCenter/clipPlaneNormal[2]/clipPlaneAxis[2]
    // = IDB x163..x177) which CullCubic reads.
    vec3_t clipWorldNormal[2] = {};   // IDB x33..x35 / x36..x38 (world-space frustum side-plane normals)
    int    clipWorldAxis[2][3] = {};  // IDB x39..x41 / x42..x44 (world-space box-corner axis selectors)
    vec3_t clipBoxCenter      = {};   // IDB x163..x165 (camera origin in the target local space)
    vec3_t clipPlaneNormal[2] = {};   // IDB x166..x168 / x169..x171 (local-space side-plane normals)
    int    clipPlaneAxis[2][3] = {};  // IDB x172..x174 / x175..x177 (local-space box-corner axis selectors)

    void Cam_BuildMatrix();           // angles → vpn/vright/vup + forward/right (0x403470)
    // 0x405460 Cam_Fov — build the WORLD-space frustum side planes (clipWorldNormal/Axis)
    // from the camera basis + camera_fov, then Cam_SetupClipPlanes(this, world_orient).
    void Cam_Fov();
    // 0x405620 sub_405620 — transform the world-space clip setup into `orient`'s local
    // space (clipBoxCenter / clipPlaneNormal / clipPlaneAxis) for CullCubic.
    void Cam_SetupClipPlanes( const float *orient );
    bool Cam_SetupScene();            // perspective R_Ed_SetSceneParms (Cam_Draw prologue); false = degenerate-projection frame dropped
    void Cam_Draw();                  // the textured world draw (0x407dc0 subset)
    // 0x4034E0 — View→Up Floor (a2=1) / Down Floor (a2=0): snap the camera onto the
    // nearest brush surface above/below it (vertical Brush_Ray march). Takes the cam
    // explicitly (the binary's handlers pass m_pCamWnd), so it is a static helper.
    static void Cam_ChangeFloor( CCamWnd *cam, int a2 );

    // ── Binary-faithful 3D-view mouse handler (0x404fc0) + its camera_mode 1/2
    //    view-control sub-handlers (0x4035f0 / 0x403700 / 0x4037c0 / 0x403870).
    //    Static (the binary's OnMouseMove passes the cam in ecx; no extra `this`).
    static void Cam_MouseMoved( CCamWnd *cam, unsigned int buttons, int x, int y );
    static void Cam_PositionDrag( CCamWnd *cam );   // 0x4035f0 (camera_mode 1, RMB)
    static void Cam_Rotate( CCamWnd *cam );         // 0x403700 (RMB+Shift+Ctrl)
    static void Cam_Rotate2( CCamWnd *cam );        // 0x4037c0 (camera_mode 2, RMB)
    static void Cam_PositionPan( CCamWnd *cam );    // 0x403870 (RMB+Ctrl)
    // 0x404d40 — RMB-release context menu (right-click brush face picker).
    static void Cam_ContextMenu( CCamWnd *cam, int x, int y );
    // The popup CMenu built by Cam_ContextMenu (IDB brushUndo@+0x334 = CMenu's CObject
    // part, context_menu@+0x338 = its m_hMenu; a single CMenu member gives both).
    CMenu m_contextMenu;

protected:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
    afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
    afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnMouseMove( UINT nFlags, CPoint point );
    afx_msg void OnDestroy();       // 0x402f10 — save "Radiant::CameraWindowPlace"
    // Context-menu WM_COMMAND handlers (message-map @ IDB 0x6d50e0):
    //   0x404c20 ContextMenuSth1  ON_COMMAND_RANGE(0x8CA0..0x8CB3) toggle-select one hit face's brush
    //   0x404cd0 OnSelectAll      ON_COMMAND(0x8CB4)  select every hit brush
    //   0x404d10 OnDeselectAll    ON_COMMAND(0x8CB5)  deselect every hit brush
    afx_msg void OnContextMenuBrushLayer( UINT nID );   // 0x404c20
    afx_msg void OnContextMenuSelectAll();              // 0x404cd0
    afx_msg void OnContextMenuDeselectAll();            // 0x404d10
    DECLARE_MESSAGE_MAP()
};

// ─── CXYWnd — the 2D grid view ───────────────────────────────────────
// A CWnd child that renders the editor's XY grid. The draw path lives in xywnd.cpp
// (XY_SetupScene / XY_DrawGrid) + the OnPaint pipeline. View-state fields mirror the
// CoD4Radiant CXYWnd members the draw path reads.
class CXYWnd : public CWnd
{
public:
    CXYWnd();

    int   m_nViewType = 2;            // ED_VIEW_XY (top-down); 0=YZ, 1=XZ, 2=XY
    float m_fScale    = 1.0f;         // IDA CXYWnd::CXYWnd 0x4637bc
    float m_vOrigin[3] = { 0.0f, 20.0f, 46.0f }; // IDA CXYWnd::CXYWnd 0x463784..0x4637a8
    int   m_nWidth    = 0;            // client width  (px), updated in OnSize
    int   m_nHeight   = 0;            // client height (px)
    bool  m_bActive   = true;

    // Input state (mirrors the CoD4Radiant CXYWnd press members).
    int    m_nButtonstate    = 0;     // current MK_ button flags during a press
    int    m_nPressx         = 0;     // press pixel X
    int    m_nPressy         = 0;     // press pixel Y
    float  m_vPressdelta[3]  = { 0, 0, 0 };
    POINT  m_ptCursor        = { 0, 0 };  // screen cursor at press (for RMB scroll)
    bool   m_bPress_selection = false;    // something was selected at press time

    // EViewType values ARE the m_nViewType values (the binary's SetViewType stores the
    // arg directly into m_nViewType): YZ Side=0, XZ Front=1, XY Top=2.
    enum EViewType { YZ = 0, XZ = 1, XY = 2 };

    // ── XY view bounds + clip planes (binary CXYWnd fields +168..+256) ─────────
    // Set by XY_SetupViewBounds (0x46cc00, called from XY_Draw @0x46cf82) and re-derived
    // into a prefab's LOCAL space by XY_SetupClipPlanes (0x46cca0) so the per-content
    // brush cull (sub_46CD80, XY_CullBrush) can reject prefab-content brushes outside the
    // 2D view rect.  Skipping the cull overruns the 16384-entry radiant_modelSkinnedSurfs[]
    // on a big map (~23k model draws/frame) and silently drops models.
    int   m_clipDim1 = 0;              // +168 (x84)  horizontal world axis
    int   m_clipDim2 = 1;              // +172 (x85)  vertical world axis
    int   m_clipDim3 = 2;              // +176 (x86)  the out-of-plane axis
    float m_clipMin1 = 0.0f;           // +180 (x87)  view-rect min along dim1
    float m_clipMin2 = 0.0f;           // +184 (x88)  view-rect min along dim2
    float m_clipMax1 = 0.0f;           // +188 (x89)  view-rect max along dim1
    float m_clipMax2 = 0.0f;           // +192 (x90)  view-rect max along dim2
    float m_clipPlanes[4][4] = {};     // +196..+256  four {normal[3], dist} view-edge planes
    void  XY_SetupViewBounds();                    // 0x46cc00 CXYWnd_SetupViewBounds
    void  XY_SetupClipPlanes( const float *orient ); // 0x46cca0 CXYWnd_SetupClipPlanes
    void Paste();
    void Copy();
    void PositionView();
    void SetViewType( EViewType vt );
    bool SetRotateMode( char bMode );   // 0x46E090 — free mouse-rotation toggle

protected:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
    // Input handlers.
    afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnMouseMove( UINT nFlags, CPoint point );
    afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnMButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnMButtonUp( UINT nFlags, CPoint point );
    afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
    afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
    afx_msg void OnDestroy();       // 0x463d00 — save "Radiant::XYWindowPlace"
    DECLARE_MESSAGE_MAP()
};

// ─── CZWnd — the vertical-ruler (Z) view ────────────────────────────────
// The Z view is the cheap vertical-ruler view (Q3Radiant "Z checker"): it draws a Z
// grid and each brush's Z-extent as a vertical bar at the camera's XY column.  It renders
// from the SAME D3D9 device as CXYWnd via its own swap chain (dx.windows[1]); the draw
// path lives in z.cpp (Z_Draw / Z_DrawGrid).
class CZWnd : public CWnd
{
public:
    CZWnd();

    int m_nWidth  = 0;     // client width  (px), mirrored to z_width  in OnSize
    int m_nHeight = 0;     // client height (px), mirrored to z_height in OnSize

protected:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
    // Z-view mouse interaction (IDB CZWnd::On* 0x46eda0/0x46f220/0x46efc0/0x46ee60/0x46f2a0).
    afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnMouseMove( UINT nFlags, CPoint point );
    afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
    DECLARE_MESSAGE_MAP()
};

// ─── CEdBlankPane — minimal render pane (layered-material content shell) ───────
// R_BeginRegistrationInternal asserts and attaches the D3D device to ALL FIVE editor
// child windows (Camera/XY/Z/Texture/LayeredMaterialWnd-content), so the layered-material
// content needs a real HWND + swap chain even before its window exists.  This pane owns
// one and clears to a solid colour each frame (the real views' OnPaint skeleton, minus
// any geometry).
class CEdBlankPane : public CWnd
{
public:
    CEdBlankPane();
    float m_clear[4] = { 0.18f, 0.18f, 0.18f, 1.0f };   // pane background
    int   m_nWidth   = 0;
    int   m_nHeight  = 0;
protected:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
    DECLARE_MESSAGE_MAP()
};

// ─── CTexWnd — the texture-browser window (4th pane) ──────────────────────────
// Renders the registered editor materials (texWndGlob.sorted_materials) as a
// scrollable 2D thumbnail grid (R_AddCmdProjectionSet2D + R_AddCmdDrawStretchPic +
// R_AddCmdDrawText — same OnPaint skeleton as the CCamWnd/CXYWnd views). Clicking a
// thumbnail selects it as the current texture and (if faces/brushes are selected)
// applies it via Brush_SetTexture.  Layout + filter/sort are documented in texwnd.cpp.
class CTexWnd : public CWnd
{
public:
    CTexWnd();
    int   m_nWidth   = 0;
    int   m_nHeight  = 0;
    int   m_scrollY  = 0;             // vertical scroll offset (pixels)
    int   m_selIndex = -1;            // selected material (index into sorted_materials)
    int   m_contentH = 0;            // last laid-out content height (for scroll clamp)
    void  DrawMaterials();           // the 2D grid draw (called inside OnPaint)
    int   HitTest( int px, int py ); // pane pixel → material index (or -1)
    void  ShowMaterialStatus( int px, int py ); // IDB TexWnd_SelectMaterial 0x45c520 (status readout)
    int   CheckScroll( int n );      // IDB CTexWnd::CheckScroll 0x45c7c0 (clamp+SetScrollPos+invalidate)
    void  UpdateScrollRange();       // IDB sub_45C830 (SetScrollInfo range from m_contentH)
    void  ApplyMaterialAtIndex( int idx );        // select+apply material idx (click body)
    BOOL  UpdatePrefs();             // IDB CTexWnd::UpdatePrefs 0x45D9F0 (re-apply prefs)
protected:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
    afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnRButtonDown( UINT nFlags, CPoint point );   // IDB CTexWnd::OnButtonDown 0x45c9a0 (right path)
    afx_msg void OnRButtonUp( UINT nFlags, CPoint point );     // IDB CTexWnd::OnButtonUp 0x45ca30 (view-mode menu)
    afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
    afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );  // IDB CTexWnd::OnVScroll 0x45dc80
    DECLARE_MESSAGE_MAP()
};

// ─── CEntityWnd — the entity inspector (win_ent.cpp / IDB CEntityWnd cluster) ──
// The QE4-lineage entity window: a control host (NOT a render pane) holding the
// eclass LIST (populated from g_eclass) and the KEY/VALUE editor (the selected
// entity's epairs, with edit/add/delete fields).  Faithful to the binary's raw-Win32
// EntityWndProc/FillClassList/SetKeyValuePairs/AddProp/DelProp/EditProp/CreateEntity/
// UpdateSelection; the window plumbing is MFC (the binary's CreateDialog(IDD_ENTITY)
// template is not in radiant.rc, so the child controls are created directly).
class CEntityWnd : public CWnd
{
public:
    CEntityWnd();
    void RefreshClassList();          // (re)populate the eclass listbox from g_eclass
    void RefreshFilterPane();         // (re)populate the Filters pane checklists (CFilterWnd)
    void LayoutForMode();             // show/hide the active mode's controls (SizeEntityDlg)
protected:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    // Route keystrokes through the main frame's accelerator while this owned popup has focus,
    // so the inspector hotkeys (F filters / N entity / O console / T textures) still fire —
    // in particular so the window's own hotkey TOGGLES IT CLOSED (the binary's OnFilterDlg etc.
    // hide the window when it is already visible in that mode).  Skipped when a text Edit has
    // focus so typing key/value text isn't eaten by a plain-letter accelerator.
    virtual BOOL PreTranslateMessage( MSG *pMsg );
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnClose();           // floating window: hide instead of destroy (CS_NOCLOSE-equiv)
    afx_msg void OnDestroy();         // 0x498253 — SaveWindowPlacement("EntityWindowPlace")
    afx_msg void OnEclassSelChange();
    afx_msg void OnEclassDblClk();
    afx_msg void OnKVSelChange();
    afx_msg void OnDeleteKey();
    afx_msg void OnSpawnFlagCheck( UINT nID );   // a spawnflag checkbox toggled
    afx_msg void OnAngleButton( UINT nID );      // an angle/direction button clicked
    afx_msg void OnAddModel();                   // the "Add Model" button → model/prefab picker
    afx_msg void OnMeasureItem( int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct ); // filter list row height
    afx_msg void OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct );           // filter list checkbox rows
    afx_msg void OnFilterFlagCheck( UINT nID );  // a simple show-flag checkbox toggled
    DECLARE_MESSAGE_MAP()
};

// CFilterWnd refresh entry — the Filters inspector pane re-reads the loaded filter
// lists + d_xyShowFlags (called on map load + when the inspector enters Filter mode).
void Radiant_RefreshFilterPane();

// SetInspectorMode (IDB CEntityWnd_SetInspectorMode 0x496b00): switch the right-column
// inspector between Entity / Textures / Console / Filters (INSPECTOR_* in qedefs.h).
// Faithful state machine (sets inspector_mode + window title + the ID_MISC_SELECTENTITYCOLOR
// menu enable) adapted to this port's separate panes: "show one pane-group at a time".
// Driven by the N/O/T/F hotkeys + View menu (no tab control — see win_ent.cpp).
void CEntityWnd_SetInspectorMode( int mode );
extern int inspector_mode;            // IDB 0x240a110 — the active inspector pane

// ─── CTextureBar — the docked texture toolbar (texturebar.cpp) ────────────────
// Shows the current/picked face's texture-alignment numbers (Shift H/V, Scale H/V, Rotate
// in raw texels/degrees) + the current material name, with spin-button quick nudges that
// forward to the already-ported texture ops (Brush_Shift/Scale/RotateTexture, select.cpp).
// The binary is a CDialogBar docked at the top of CMainFrame; radiant.rc has no template,
// so it is HAND-BUILT (the CVehicleDlg pattern) as a child bar across the top strip.
// GetSurfaceAttributes (0x459610) is THE REFRESH ENTRY — called wherever the current
// texture / selection changes (UpdateTextureBar / Material_SetMode / CTexWnd::OnButtonDown /
// map load).  Embedded by value in CMainFrame as m_wndTextureBar.
class CTextureBar : public CWnd
{
public:
    CTextureBar();
    static void GetSurfaceAttributes( CTextureBar *bar );   // 0x459610 — read texdef → display
    // Create the bar across the top of the frame; returns its height (layout inset).
    static int  CreateBar( CWnd *frame, CTextureBar *bar, int frameWidth );
    static void LayoutBar( int frameWidth );                // reposition on frame resize
protected:
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct ); // 0x459250 — build child controls
    afx_msg void OnDeltaPos( NMHDR *pNMHDR, LRESULT *pResult ); // the five spin nudges
    afx_msg void OnFieldEdited();                           // EN_KILLFOCUS → ApplyFields
    void ApplyFields();                                     // TextureBar_02 (0x459750) core
    DECLARE_MESSAGE_MAP()
};

// Texture-bar height consumed at the top of the frame (0 when the bar isn't up).
extern int g_texBarHeight;

// ─── CSurfaceDlg — the Surface Inspector (surfacedlg.cpp / IDB SurfaceInspector cluster) ─
// Edit a selected face's/patch's texture alignment (shift X/Y, stretch X/Y, rotate) with a
// live camera update.  A modeless CDialog on IDD_SURFACE_INSPECTOR (116) — exactly the binary
// (CDialog::Create(IDD_SURFACE_INSP) in DoSurface 0x4585d0) — opened by Textures→Surface
// Inspector (menu 33041).  The template + control ids are the binary's (res/radiant.rc).
// Handlers are 1:1 ports of the CSurfaceDlg cluster; the edit core (SurfaceInspector_GetMaterialDef
// read-fields→texdef + GetTexMods commit + Select_SetTexture_2 refresh) lives in surfacedlg.cpp.
class CSurfaceDlg : public CDialog
{
public:
    CSurfaceDlg();
    // flag byte members read by GetTexMods (binary this->xx9_1 / xx9_2 — "texdef edited" /
    // "sample-size edited" dirty flags set by the edits' EN_CHANGE handlers).
    int  m_bPatchMode;        // binary this+116 (m_bPatchMode) — patch vs brush/face selection
    char m_texdefDirty;       // xx9_1: a shift/stretch/rotate edit changed
    char m_sampleDirty;       // xx9_2: the sample-size edit changed

protected:
    virtual void DoDataExchange( CDataExchange *pDX );   // 0x456c00
    virtual BOOL OnInitDialog();                          // 0x458710
    virtual void PostNcDestroy();
    // Modeless close routing: Enter→OnOK, Esc→OnCancel, [X]→OnClose all funnel to a real
    // DestroyWindow (which fires OnDestroy → clears g_surfwin, then PostNcDestroy → clears
    // g_pSurfDlg + delete this).  Without these overrides the CDialog defaults call
    // ::EndDialog, which on a MODELESS dialog only HIDES it and never fires OnDestroy — so
    // g_surfwin/g_pSurfDlg stayed set and the inspector reappeared on the next update.
    virtual void OnOK();         // Done / Enter — commit pending edits, then close
    virtual void OnCancel();     // Cancel / Esc / [X] — revert scratch, then close
    afx_msg void OnClose();      // WM_CLOSE ([X] title button) → OnCancel
    afx_msg void OnDone();       // "Done" button 1489 (0x4589e0) → OnOK
    afx_msg void OnCancel2();    // "Cancel" button 1198 (0x458ab0) → OnCancel
    afx_msg void OnFit();        // "Fit" 1286 → Brush_FitTexture (0x458DE0)
    afx_msg void OnNaturalize(); // "Natural" 1284 patch natural projection  (0x458C60)
    afx_msg void OnCap();        // "CAP" 1282 patch cap texturing           (0x458B40)
    afx_msg void OnLightmap();   // "Lmap" 1287 patch lightmap alignment     (0x458CA0)
    afx_msg void OnSet();        // "Set..." 1283 patch 2D/3D/Curve texturing (0x458CB0)
    afx_msg void OnDeltaPosSpin( NMHDR *pNMHDR, LRESULT *pResult ); // spin arrows (0x458b10 → UpdateSpinners 0x457cf0)
    afx_msg void OnTexdefEdited();   // EN_CHANGE on shift/stretch/rotate edits (0x458e10): xx9_1=1
    afx_msg void OnSampleEdited();   // EN_CHANGE on sample-size edit           (0x458e20): xx9_2=1
    afx_msg void OnDestroy();        // 0x458a50: save window rect + clear g_surfwin
    DECLARE_MESSAGE_MAP()
};

// surfacedlg.cpp entry points (menu open + selection/load refresh).
void Surf_OpenInspector();      // DoSurface (0x4585d0) — open/show the inspector
void Surf_UpdateInspector();    // UpdateSurfaceDialog (0x458590) — refresh fields on change

// ─── CFindTextureDlg — Find / Replace Texture(s) (findtexture.cpp / IDB cluster) ─
// Swap a material name across selected (or all active) brush faces.  A hand-built
// modeless popup CWnd (the binary's modeless CDialog IDD_DIALOG_FINDREPLACE_TEXTURE;
// radiant.rc has no template), opened by Textures→Replace All (menu 32812 →
// CMainFrame::OnTextureReplaceall → CFindTextureDlg::show 0x415c20).  The two edit
// fields (Find / Replace) + three checkboxes (selected-only / force-replace-all /
// live-mapping) build the flag byte for FindReplaceTextures (findtexture.cpp).
class CFindTextureDlg : public CWnd
{
public:
    CFindTextureDlg();
    static void show();          // 0x415c20 — open/show the singleton
    // 0x415ce0 / 0x415d40 — push a picked material name into the Find / Replace edit
    // field (Texture_SetTexture's find-dialog-visible branch).  Faithful to the IDB's
    // UpdateData(TRUE) → if(control populated) str_set(member) → UpdateData(FALSE).
    void SetFindText( const char *name );     // sub_415CE0 (str_set @0x78 — Find)
    void SetReplaceText( const char *name );  // sub_415D40 (str_set @0x7C — Replace)
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnFindReplaceOK();      // OK   — replace + close
    afx_msg void OnFindReplaceApply();   // Apply— replace, stay open
    afx_msg void OnFindReplaceClose();   // Close— just close
    afx_msg void OnFindSetFocus();       // sub_415DF0 — Find edit focus  → byte_73C380=1
    afx_msg void OnReplaceSetFocus();    // sub_415E00 — Replace edit focus → byte_73C380=0
    DECLARE_MESSAGE_MAP()
};

// ─── CLayerDlg (the Layers management dialog; layersdlg.cpp) ──────────────────
// Hand-built modeless popup: a CListBox of layers + command buttons.  Toggle()
// is the CMainFrame::OnLayersDlg target (menu 33954).
class CLayerDlg : public CWnd
{
public:
    CLayerDlg();
    static void Toggle();        // 0x42BD10 — open/show or hide the singleton
    void Refresh();              // repopulate the layer list from the layerMap
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnClose();
    afx_msg void OnDestroy();       // 0x41C1B0 — save "Radiant::LayersDlgPlace"
    afx_msg void OnNew();
    afx_msg void OnDelete();
    afx_msg void OnRename();
    afx_msg void OnAssign();
    afx_msg void OnSelectAll();
    afx_msg void OnMakeActive();
    afx_msg void OnHide();
    afx_msg void OnShow();
    // The binary's MENU233 right-click context menu (CLayerDlg::ContextMenu 0x41DEF0 +
    // the 14 CLayersDlg WM_COMMAND handlers @0x6D8920).  See layersdlg.cpp.
    afx_msg void OnContextMenu( CWnd *pWnd, CPoint point );
    DECLARE_MESSAGE_MAP()
};

// ─── CDynEntityDlg (the Dyn-Entity authoring dialog; dynentitydlg.cpp) ────────
// Hand-built modeless popup: a type combobox + 4 key edits (health/physPreset/
// destroyEfx/destroyPieces) with per-field Set/Clear/Browse + Help.  Toggle() is the
// CMainFrame::OnMiscDynEntities target (menu 36106 / 0x8D0A).  Each Set pushes the
// field onto the selected dyn_ entities (DynEntityDlg_01_SetPair); Clear removes the
// key (DynEntityDlg_02_RemovePair); a write-only authoring dialog (no OnInitDialog).
class CDynEntityDlg : public CWnd
{
public:
    CDynEntityDlg();
    static void Toggle();        // 0x42BD90 — open/show or hide the singleton
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnSetType();
    afx_msg void OnSetHealth();
    afx_msg void OnSetPhys();
    afx_msg void OnSetEfx();
    afx_msg void OnSetPieces();
    afx_msg void OnClearType();
    afx_msg void OnClearHealth();
    afx_msg void OnClearPhys();
    afx_msg void OnClearEfx();
    afx_msg void OnClearPieces();
    afx_msg void OnBrowsePhys();
    afx_msg void OnBrowseEfx();
    afx_msg void OnBrowsePieces();
    afx_msg void OnHelp();
    DECLARE_MESSAGE_MAP()
};

// ─── CVehicleDlg (the Vehicle-Group authoring dialog; vehicledlg.cpp) ─────────
// Hand-built modeless popup: 4 key edits (script_startinghealth / script_accuracy /
// speed / lookahead) with per-field Set/Clear, a crash-type combo, and on/off toggle
// buttons for the boolean keys (script_deathroll / script_turret / script_turretmg /
// script_badplace / script_avoidvehicles / script_attackai / script_team).  Toggle()
// is the CMainFrame::OnMiscVehicleGroup target (menu 33221 / 0x81C5).  Each Set pushes
// the field/state onto the selected vehicle entities (VehicleDlg_SetPair); Clear removes
// the key (VehicleDlg_RemovePair); a write-only authoring dialog (no OnInitDialog),
// applies to any selected non-world entity (vehicle / vehicle path node — no eclass
// filter, unlike the dyn_ filter in CDynEntityDlg).
class CVehicleDlg : public CWnd
{
public:
    CVehicleDlg();
    static void Toggle();        // 0x42BD50 — open/show or hide the singleton
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnSetHealth();
    afx_msg void OnSetAccuracy();
    afx_msg void OnSetSpeed();
    afx_msg void OnSetLookahead();
    afx_msg void OnClearHealth();
    afx_msg void OnClearAccuracy();
    afx_msg void OnClearSpeed();
    afx_msg void OnClearLookahead();
    afx_msg void OnSetCrashType();
    afx_msg void OnClearCrashType();
    afx_msg void OnToggle( UINT nID );   // an on/off toggle pushbutton (ranged)
    afx_msg void OnScriptGroup( UINT nID );  // a script-group button (ranged) → AssignNextNumber
    DECLARE_MESSAGE_MAP()
};

// ─── CModelDlg (the "Replace Models" tool dialog; modeldlg.cpp) ───────────────
// Hand-built modeless popup: two list boxes (FROM / TO model sets) with per-list
// [Add file...] / [Remove], three class checkboxes (misc_model / dyn_model / misc_prefab),
// and [Replace].  Toggle() is the CMainFrame::OnReplaceModels target (menu 33240 / 0x81D8).
// [Replace] mass-swaps every matching entity's "model" key for a random TO entry
// (ModelDlg_DoReplace).  A bulk model replacer/randomiser; the picker UI rides a plain
// CFileDialog (the CModelFileDialog preview-pane subclass is parked, as in win_ent.cpp).
class CModelDlg : public CWnd
{
public:
    CModelDlg();
    static void Toggle();        // 0x42BF00 — open/show or hide the singleton
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnAddFrom();
    afx_msg void OnRemoveFrom();
    afx_msg void OnAddTo();
    afx_msg void OnRemoveTo();
    afx_msg void OnReplace();
    DECLARE_MESSAGE_MAP()
};

// ─── CVertEditDlg (the patch Vertex Color/Alpha editor; verteditdlg.cpp) ──────
// Hand-built modeless popup: 4 R/G/B/A sliders, a [Color...] picker, two enable checkboxes
// (Color / Alpha) and [Apply].  Toggle() is the CMainFrame::OnVertexEditDlg target (menu
// 33199 / 0x81AF, accel 'G').  [Apply] paints the chosen RGBA onto the currently-selected
// patch control points (the d_move_points set, vertex/edge mode), with Undo + Patch_Rebuild
// (VertEditDlg_Apply).
class CVertEditDlg : public CWnd
{
public:
    CVertEditDlg();
    static void Toggle();        // 0x42BCD0 — open/show or hide the singleton
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
    afx_msg void OnColorButton();
    afx_msg void OnApply();
    DECLARE_MESSAGE_MAP()
};

// ─── CMapInfo (the read-only Map Info dialog; mapinfo.cpp) ────────────────────
// Edit→Map Info... (menu 32786 / 0x8012 → CMainFrame::OnEditMapinfo 0x426C60).  A
// snapshot of the loaded map's geometry/entity statistics: a 9-row × 3-column grid
// (World / Prefabs / Total) of count statics (ctrl ids 1490..1516) + a listbox (id 1013)
// of each entity classname with its instance count.  The binary is a MODAL CDialog
// (template IDD_DLG_MAPINFO=128); radiant.rc has no template, so it is HAND-BUILT (the
// CVehicleDlg pattern) and recomputed per open.  Counts via MapInfo_01; per-class list
// via MapInfo_02.  Read-only — no input controls, no key writing.
class CMapInfo : public CWnd
{
public:
    CMapInfo();
    static void Show();          // 0x426C60 — (re)create + populate + show the popup
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    DECLARE_MESSAGE_MAP()
};

// ─── CEntityListDlg — the Entity List browser (entitylist.cpp / EntityListDlg::*) ─
// Edit→Entity Info... (menu 32787 → CMainFrame::OnEditEntityinfo 0x426D6F).  A tree of
// every map entity grouped by classname (SysTreeView32 id 1022) + a Key/Value list for
// the selected entity (SysListView32 id 1025); selecting/double-clicking a leaf selects
// that entity's brushes in the views.  The binary is a MODAL CDialog (template
// IDD_DLG_ENTITYLIST=129); radiant.rc has no template, so it is HAND-BUILT (the CMapInfo
// pattern) and re-populated per open.  The class itself is private to entitylist.cpp; only
// the open function is exposed.
void EntList_Open();                     // 0x426D6F path — (re)create + populate + show

// ─── CFindBrushDlg (Misc→Find brush / select by number; win_dlg.cpp) ──────────
// Misc→Find brush (menu 33023 / 0x8117 → CMainFrame::OnMiscFindbrush 0x424B96).  Two
// numeric edits (entity #, brush #) preloaded from the current selection
// (GetSelectionIndex); Find selects that brush by number (Select_ByEntityNumber).  The
// binary is a MODAL DialogBoxParamA(IDD_FINDBRUSH, FindBrushDlgProc@0x4957E0); radiant.rc
// has no template, so it is HAND-BUILT (the CMapInfo pattern) as a modeless popup with
// explicit Find/Cancel buttons.
class CFindBrushDlg : public CWnd
{
public:
    CFindBrushDlg();
    static void Show();          // (re)create + preload + show the popup
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnFind();       // IDOK — Select_ByEntityNumber(brush#, ent#)
    afx_msg void OnCancel2();    // IDCANCEL — just hide
    DECLARE_MESSAGE_MAP()
};

// ─── CGoToDlg (Misc→Go to position; win_dlg.cpp) ──────────────────────────────
// Misc→Go to position (menu 33107 / 0x8163 → CMainFrame::OnMiscGoToPosition 0x424BB9).
// One free-text edit; Go parses up to four coordinate formats ("x, y, z", "(x, y, z)",
// "x y z", "(x y z)") and moves the camera + XY view there (Select_Deselect +
// CXYWnd::PositionView).  The binary is a MODAL DialogBoxParamA(IDD_DIALOG_GOTO_POS,
// GoToDlgProc@0x495980); HAND-BUILT here as a modeless popup with Go/Cancel buttons.
class CGoToDlg : public CWnd
{
public:
    CGoToDlg();
    static void Show();          // (re)create + show the popup
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnGo();         // IDOK — parse coords → move camera/XY view
    afx_msg void OnCancel2();    // IDCANCEL — just hide
    DECLARE_MESSAGE_MAP()
};

// ─── CArbRotateDlg (Selection→Rotate→Arbitrary; win_dlg.cpp) ──────────────────
// Selection→Rotate→Arbitrary (menu 33033 / 0x8109 → CMainFrame::OnSelectionArbitrary-
// rotation 0x425300, which constructs CRotateDlg@0x450B10 + DoModal).  Three numeric
// edits (X/Y/Z angle in degrees); OK applies each non-zero angle as an axis rotation to
// the selection (Select_GetMid → Select_RotateAxis → Select_ApplyMatrix_SelectedBrushes
// — the same matrix path as Brush→Rotate→X/Y/Z), undo-bracketed.  HAND-BUILT here as a
// modeless popup with OK/Cancel buttons.
class CArbRotateDlg : public CWnd
{
public:
    CArbRotateDlg();
    static void Show();          // (re)create + show the popup
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnApply();      // IDOK — rotate the selection by the typed X/Y/Z angles
    afx_msg void OnCancel2();    // IDCANCEL — just hide
    DECLARE_MESSAGE_MAP()
};

// ─── CAboutDlg (Help→About; win_dlg.cpp) ──────────────────────────────────────
// Help→About (menu 33038 / 0x8106 → CMainFrame::OnHelpAbout 0x4264F0, which does
// DialogBoxParamA(IDD_ABOUTBOX, AboutDlgProc@0x496300)).  AboutDlgProc is trivial: an
// info box that EndDialogs on OK / close.  radiant.rc has no template (the Phase-0
// extractor took only menus/accels), so it is HAND-BUILT (the CMapInfo pattern) as a
// modeless popup showing the editor name/version with a single OK button.
class CAboutDlg : public CWnd
{
public:
    CAboutDlg();
    static void Show();          // (re)create + show the popup
protected:
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnOk2();        // IDOK — just hide (EndDialog(,1) in the binary)
    DECLARE_MESSAGE_MAP()
};

// DoColor (win_dlg.cpp, IDB 0x499350) — the Colors-menu colour picker.  Seeds a CColorDialog
// from g_qeglobals.d_savedinfo.colors[index], on OK writes the chosen RGB back (alpha = 1)
// and flags a full window update.  Returns 1 on OK, 0 on cancel.
int DoColor( int index );

// ─── CMainFrame ───────────────────────────────────────────────────────────────
class CMainFrame : public CFrameWnd
{
public:
    CMainFrame();
    virtual ~CMainFrame();

    // Look the pressed key (+ current modifiers) up in the editor command map and dispatch its
    // WM_COMMAND.  Returns true if a command fired.  Used by OnKeyDown and by the floating
    // inspector popup (CEntityWnd) so its hotkeys (F/N/O/T) reach the command map even when the
    // popup — not a view — has focus.  (Editor hotkeys are in g_radiantCommands, NOT m_hAccel.)
    bool TryHotkey( UINT nChar );

    // Window children (nullptr until initialized)
    CCamWnd      *m_pCamWnd   = nullptr;
    CXYWnd       *m_pXYWnd    = nullptr;
    CXYWnd       *m_pActiveXY = nullptr;  // whichever XY view has focus
    CZWnd        *m_pZWnd     = nullptr;  // Z (vertical-ruler) view
    CTexWnd      *m_pTexWnd   = nullptr;  // texture browser (right pane)
    CEdBlankPane *m_pLayMat   = nullptr;  // layered-material content (hidden render shell)
    CEntityWnd   *m_pEntWnd   = nullptr;  // entwnd: the entity inspector (d_hwndEntity)
    CEdit         m_wndConsole;           // console: the editor log pane (g_qeglobals.d_hwndEdit)
    CStatusBar    m_wndStatusBar;         // status bar (brush/entity counts)
    CToolBar      m_wndToolBar;           // the main icon toolbar (IDR_TOOLBAR152)
    CTextureBar   m_wndTextureBar;        // embedded texture bar
    bool          m_bDoLoop = false;      // RoutineProcessing gate (set once init is up)

    // Map-editing handlers
    void OnPrefabEnter();     // Edit→Enter Prefab (33173) — body in map.cpp
    void OnPrefabLeave();
    bool ConfirmModified();   // 0x49a030 — the "Save changes first?"/"Lose changes?" prompt
    bool OkToDiscard();       // the binary's per-command guard (silent if clean, else prompt)
    bool OnFileSaveAs_Confirmed(); // OnFileSaveAs body, returns saved(true)/cancelled(false)

    // The invalidation broadcast (CMainFrame::UpdateWindows 0x427090): RedrawWindow each
    // view whose W_* bit is set.  Sys_UpdateWindows accumulates bits into g_nUpdateBits;
    // RoutineProcessing (driven from CRadiantApp::OnIdle) flushes them through here.
    void UpdateWindows( int nBits );
    void RoutineProcessing();             // idle pump: flush g_nUpdateBits

    // Editor hotkey dispatch (0x422370): look the pressed key+modifiers up in the built-in
    // command map (g_radiantCommands) and SendMessage its WM_COMMAND. Called directly from
    // the views' OnKeyDown (CXYWnd/CCamWnd/CZWnd), exactly as the binary tail-calls it.
    void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );

    // 0x421230 / 0x420460 — the user-remappable keyboard command map.  LoadCommandMap layers
    // radiant.ini [Commands] overrides on top of the built-in defaults; ShowMenuItemKeyBindings
    // annotates each menu item with its current key binding.  Both called from OnCreate.
    void LoadCommandMap();
    BOOL ShowMenuItemKeyBindings( HMENU hMenu );

    // 0x4298B0 — nudge the selection by fNudge along axis nDim (curve-point path uses
    // MoveSelection; else Select_Move).  Called by NudgeSelection (select.cpp).
    void Nudge( int nDim, float fNudge );

    // File / Edit command wrappers.
    afx_msg void OnFileNew();
    afx_msg void OnFileOpen();
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
    afx_msg void OnFileExit();
    afx_msg void OnFileNewproject();        // 0x426E80 — File→New Project    (32791)
    afx_msg void OnFileProjectsettings();   // 0x428DE0 — File→Project Settings(32818)
    afx_msg void OnSetStartupProject();     // 0x427010 — File→Set Startup Project (35018)
    afx_msg void OnMru( UINT nID );         // 0x423FE0 — File→Recent Files (8000-8009)
    afx_msg void OnDestroy();               // 0x421C60 — SaveMruInReg on shutdown
    afx_msg void OnPointfileOpen();         // 0x423b20 — File→Pointfile toggle (32967)
    afx_msg void OnMiscNextleakspot();      // 0x424bc0 — Misc→Next leak spot (33024)
    afx_msg void OnMiscPreviousleakspot();  // 0x424be0 — Misc→Previous leak spot (33025)
    afx_msg void OnMiscPrintxy();           // 0x424c00 — Misc→Print XY View (33037, WXY_Print)
    afx_msg void OnSelectionPrint();        // 0x429110 — Selection→Print (33087, Brush_Print)
    afx_msg void OnClose();             // 0x422220 — WM_CLOSE: ConfirmModified exit guard
    afx_msg void OnEditUndo();
    afx_msg void OnEditRedo();
    afx_msg void OnEditCopybrush();    // 0x4286B0 — Edit→Copy  (33039)
    afx_msg void OnEditPastebrush();   // 0x4286D0 — Edit→Paste (33040)

    // Grid menu (11 grid-size items). OnGridSize is the IDB ranged handler OnGrid1
    // (0x424ab0); SetGridStatus writes the grid text to status pane 4 (0x428a00).
    afx_msg BOOL OnGridSize( UINT nID );
    void SetGridStatus();

    // View + Selection menu wrappers; each forwards to a ported core.  IDB EAs in
    // mainfrm.cpp.
    afx_msg void OnShowRegionsForSelected(); // 0x4241E0 — build light-preview CSG regions (cmd 36125, Shift+F8)
    afx_msg void OnSetAsActiveLayer();       // 0x42BFD0 — Layers "Set as active layer" (cmd 33955, empty stub)

    // ── UI COMMAND-WIRING batch — thunks whose cores are already ported. ──
    afx_msg void OnCameraLeft();             // 0x426720 (33057)
    afx_msg void OnCameraRight();            // 0x426770 (33058)
    afx_msg void OnCameraForward();          // 0x4266B0 (33059)
    afx_msg void OnCameraBack();             // 0x426610 (33060)
    afx_msg void OnCameraStrafeleft();       // 0x4267C0 (33063)
    afx_msg void OnCameraStraferight();      // 0x426820 (33064)
    afx_msg void OnGridToggle();             // 0x426930 (33065)
    afx_msg void OnGridNext();               // 0x4289A0 (33083)
    afx_msg void OnGridPrev();               // 0x4289D0 (33084)
    afx_msg void OnSelectionTextureSnapDec();// 0x4287B0 (33072)
    afx_msg void OnSelectionTextureSnapInc();// 0x428830 (33073)
    afx_msg void OnSelectionTextureFitUnk(); // 0x4287D0 (33074)
    afx_msg void OnTextureFitAll();          // 0x428800 (33234)

    // ── TEXTURE REFRESH / RESOLUTION / WINDOW-SCALE (Textures menu) ──────────────
    afx_msg void OnTextureRefresh();         // 0x428B50 (33204, F5) — R_ReloadImages
    afx_msg void OnTextureResolution( UINT nID ); // 0x424340 (36115..36118) — PicMip+R_UpdateMipMap+R_ReloadImages
    afx_msg void OnTexturesTexturewindowscale10();  // 0x42AFE0 (32898)
    afx_msg void OnTexturesTexturewindowscale25();  // 0x42B040 (32897)
    afx_msg void OnTexturesTexturewindowscale50();  // 0x42B060 (32896)
    afx_msg void OnTexturesTexturewindowscale100(); // 0x42B000 (32895)
    afx_msg void OnTexturesTexturewindowscale200(); // 0x42B020 (32894)
    void PicMip();                           // 0x420860 — apply d_picmip → r_picmip* + check menu radio
    void CheckTextureScale( UINT uIDCheckItem ); // 0x42AF50 — check scale radio + reset browser scroll
    afx_msg void OnTexRotateClockwise();     // 0x428850 (33075)
    afx_msg void OnTexRotateCounterCW();     // 0x428860 (33076)
    afx_msg void OnTexShiftLeft();           // 0x4288E0 (33079)
    afx_msg void OnTexShiftRight();          // 0x428910 (33080)
    afx_msg void OnTexShiftUp();             // 0x428870 (33081)
    afx_msg void OnTexShiftDown();           // 0x4288A0 (33082)
    afx_msg void OnViewZ100();               // 0x424740 (32998, empty stub)
    afx_msg void OnViewZzoomin();            // 0x424A00 (32999)
    afx_msg void OnViewZzoomout();           // 0x424A40 (33000)
    afx_msg void OnViewCubeout();            // 0x428F50 (32819)
    afx_msg void OnViewCubein();             // 0x428F10 (32820)
    afx_msg void OnViewXy();                 // 0x424710 (32772)
    afx_msg void OnViewYz();                 // 0x423FB0 (32774)
    afx_msg void OnViewXz();                 // 0x424A80 (32773)
    afx_msg void OnToggleviewYz();           // 0x427230 (32797, empty stub)
    afx_msg void OnToggleviewXz();           // 0x427220 (32798, empty stub)
    // View→Toggle→{Console,Camera,XY,Z} — show/hide the docked child windows.
    afx_msg void OnToggleconsole();          // 0x426A90 (33068)
    afx_msg void OnTogglecamera();           // 0x426A40 (33069, Shift+Ctrl+C)
    afx_msg void OnTogglez();                // 0x426B30 (33070, Shift+Ctrl+Z)
    afx_msg void OnToggleview();             // 0x426AE0 (33071, Shift+Ctrl+V)
    afx_msg void OnViewNextview();           // 0x426DB0 (32789)
    afx_msg void OnToolbarMain();            // 0x4290F0 (32830, empty stub)
    afx_msg void OnToolbarTexture();         // 0x429100 (32832, empty stub)
    afx_msg void OnCenter2DOnCamera();       // 0x42A2D0 (33108)
    afx_msg void OnSelectNext();             // 0x423C90 (33160)
    afx_msg void OnSelectPrev();             // 0x423CA0 (33161)
    afx_msg void OnSelectionMovedown();      // 0x429050 (32829)
    afx_msg void OnSelectionMoveup();        // 0x4290B0 (32831)
    afx_msg void OnSelectionSelectNudgeleft();  // 0x429510 (32847)
    afx_msg void OnSelectionSelectNudgeright(); // 0x429530 (32848)
    afx_msg void OnSelectionSelectNudgeup();    // 0x429550 (32849)
    afx_msg void OnSelectionSelectNudgedown();  // 0x4294F0 (32850)
    afx_msg void OnSplay();                  // 0x4254F0 (33157)
    afx_msg void OnSetViewToEntity();        // 0x425540 (33210)
    afx_msg void OnLinkSelected();           // 0x425500 (33211)
    afx_msg void OnDistanceBetweenEntities();// 0x4294D0 (33178)
    afx_msg void OnSelectAllOfType();        // 0x42B470 (33093)
    afx_msg void OnSelectAllOfTypeRecursive();// 0x42B4B0 (33212)
    afx_msg void OnHideSelected();           // 0x42B6A0 (32923)
    afx_msg void OnHideUnselected();         // 0x42B6C0 (32934)
    afx_msg void OnShowHidden();             // 0x42B6D0 (32924)
    afx_msg void OnShowLastHidden();         // 0x42B6E0 (33246)
    afx_msg void OnViewCrosshair();          // 0x42B690 (33100)
    afx_msg void OnSelectionNoOutline();     // 0x425630 (33103)
    afx_msg void OnSelectionNoTint();        // 0x425650 (33172)
    afx_msg void OnSelectSameTargetname();   // 0x42ADA0 (36121)
    afx_msg void OnSelectSameTarget();       // 0x42ADD0 (36123)

    afx_msg void OnViewZoomin();          // 0x424750 — View→Zoom→XY Zoom In
    afx_msg void OnViewZoomout();         // 0x4247e0 — View→Zoom→XY Zoom Out
    afx_msg void OnView100();             // 0x423c30 — View→Zoom→XY 100%
    afx_msg void OnSelectionDeselect();   // 0x425740 — Selection→Deselect
    afx_msg void OnSelectionDragVertices(); // 0x425840 — Selection→Drag Vertices (vertex mode, 33005)
    afx_msg void OnSelectionDragEdges();    // 0x4257d0 — Selection→Drag Edges (edge mode, 33006)
    afx_msg void OnSelectionMakehollow(); // 0x425570 — Selection→CSG→Hollow
    afx_msg void OnSelectionCsgmerge();   // 0x4255d0 — Selection→CSG→Merge
    afx_msg void OnDropSelected();        // 0x425be0 — Selection→Drop to Floor (ID 33183)

    // CLIPPER (View→Toggle Clipper / Clip selected / Flip Clip orientation).
    afx_msg void OnViewClipper();         // 0x426510 — toggle clip mode (ID 32783)
    afx_msg void OnClipSelected();        // 0x427170 — commit the clip / Enter (ID 32795)
    afx_msg void OnFlipClip();            // 0x427140 — flip kept side (ID 32796)

    // Region menu (whole category) + Edit→Leave Prefab; each forwards to a map.cpp core.
    afx_msg void OnRegionOff();           // 0x4252b0 — Region→Off
    afx_msg void OnRegionSetxy();         // 0x4252f0 — Region→Set XY
    afx_msg void OnRegionSettallbrush();  // 0x4252e0 — Region→Set Tall Brush
    afx_msg void OnRegionSetbrush();      // 0x4252c0 — Region→Set Brush
    afx_msg void OnRegionSetselection();  // 0x4252d0 — Region→Set Selected Brushes

    // Edit→Preferences + the toggles that flip + persist the real prefs (snap /
    // texture-lock).  IDB EAs in mainfrm.cpp.
    afx_msg void OnPrefs();               // 0x426950 — Edit→Preferences...
    afx_msg void OnSnaptogrid();          // 0x428380 — Grid→Snap to grid (toggle m_bNoClamp)
    afx_msg void OnToggleLockMoves();     // 0x426b80 — Texture Lock→Moves (m_bTextureLock)
    afx_msg void OnToggleLockRotations(); // 0x429230 — Texture Lock→Rotations (m_bRotateLock)
    afx_msg void OnToggleLockLightmap();  // 0x426bf0 — Texture Lock→Lightmaps (m_bLightmapLock)

    // surfinsp — Textures→Surface Inspector (menu 33041 → DoSurface 0x4585d0).
    afx_msg void OnTexturesInspector();   // 0x424b60
    // patchinsp — Patch→Inspector (menu 33092 / Shift+S → DoPatchInspector 0x436d30).
    afx_msg void OnPatchInspector();      // 0x42b460

    // find/replace — Textures→Replace All (menu 32812 → CFindTextureDlg::show 0x415c20).
    afx_msg void OnTextureReplaceall();   // 0x428b40 (thunk → CFindTextureDlg::show)

    // Layers dialog — Layers→Layers... (menu 33954 → CLayerDlg::Toggle, layersdlg.cpp).
    afx_msg void OnLayersDlg();           // 0x42bd10 (toggle the Layers dialog visibility)
    afx_msg void OnSelectionAddToActiveLayer(); // ctx-menu "Add selection to active layer" (CXYWnd 0x466930, nID 35001)

    // Dyn-Entity dialog — Misc→Dyn Entities (menu 36106 → CDynEntityDlg::Toggle).
    afx_msg void OnMiscDynEntities();     // 0x42bd90 (toggle the Dyn-Entity dialog visibility)

    // Vehicle-Group dialog — Misc→Vehicle Group (menu 33221 → CVehicleDlg::Toggle).
    afx_msg void OnMiscVehicleGroup();    // 0x42bd50 (toggle the Vehicle-Group dialog visibility)

    // Replace-Models dialog — (menu 33240 / 0x81D8 → CModelDlg::Toggle, modeldlg.cpp).
    afx_msg void OnReplaceModels();       // 0x42bf00 (toggle the Replace-Models tool dialog)

    // Vertex-Edit dialog — Patch vertex colour/alpha (menu 33199 / 0x81AF, accel 'G' →
    // CVertEditDlg::Toggle, verteditdlg.cpp).
    afx_msg void OnVertexEditDlg();       // 0x42bcd0 (toggle the Vertex-Edit dialog visibility)

    // Map Info dialog — Edit→Map Info (menu 32786 → CMapInfo::Show, mapinfo.cpp).
    afx_msg void OnEditMapinfo();         // 0x426c60 (open the read-only Map Info dialog)

    // Entity List dialog — Edit→Entity Info (menu 32787 → EntList_Open, entitylist.cpp).
    afx_msg void OnEditEntityinfo();      // 0x426d6f (open the entity-browser tree+K/V dialog)

    // Brush→Primitives — reshape the selected brush (brush.cpp Brush_MakeSided*).
    afx_msg void OnBrushArbitrarysided(); // 0x424EE0 — Brush→Arbitrary sided cylinder (ID 33034)
    afx_msg void OnBrushMakecone();       // 0x429170 — Brush→Primitives→Cone (ID 32833)
    afx_msg void OnBrushPrimitivesSphere(); // 0x42B630 — Brush→Primitives→Sphere (ID 32892)

    // Selection menu batch — forward to ported select.cpp/entity.cpp cores.
    afx_msg void OnSelectConneted();       // 0x425550 — Select Connected (33134)
    afx_msg void OnSelectionTargetname();  // 0x426390 — Select Targetname (33132)
    afx_msg void OnSelectionClassname();   // 0x4263A0 — Select Classname (202)
    afx_msg void OnSelectionUngroupentity();// 0x426380 — Ungroup entity (33035)
    afx_msg void OnSelectionMakeDetail();  // 0x4261C0 — Make Detail (33042)
    afx_msg void OnSelectionMakeStructural();// 0x426200 — Make Structural (33043)
    afx_msg void OnSelectionCompleteTall();// 0x426340 — Select Complete Tall (32984)
    afx_msg void OnSelectionPartialTall(); // 0x426360 — Select Partial Tall (32983)
    afx_msg void OnSelectionTouching();    // 0x426370 — Select Touching (32986)
    afx_msg void OnSelectionInside();      // 0x426350 — Select Inside (33008)
    afx_msg void OnSelectionKeyValue();    // 0x4263B0 — Select by Key/Value (33133)

    // Menu-wrapper batch (wiring-only) — thin On* handlers forwarding to cores that
    // are ALREADY fully ported, so the auto-greyed menu items become live. Each is
    // transcribed from the IDB (EAs noted); the undo-bracketed ones mirror the binary
    // exactly. Siblings whose cores are still FATAL stubs / no-op (Clone, Flip/Rotate,
    // Up/Down Floor, the Select-Tall/Touching/Inside group, Show In Use) stay GRAYED.
    afx_msg void OnSelectionDelete();      // 0x425690 — Edit→Delete (33003)
    afx_msg void OnSelectionAutoCaulk();   // 0x425600 — Selection→CSG→Auto Caulk (33220)
    afx_msg void OnSelectionMakeWeaponclip();    // 0x426240 — Make Weapon Clip (196)
    afx_msg void OnSelectionMakeNonColliding();  // 0x426280 — Make Non-Colliding (197)
    afx_msg void OnSelectionMakeSplitCoplanar(); // 0x4262C0 — Make Split Coplanar Geo (33223)
    afx_msg void OnSelectionMakeDontSplitCoplanar();// 0x426300 — Make Don't Split Coplanar Geo (33224)
    afx_msg void OnViewCenter();           // 0x423C50 — View→Center (32953)
    afx_msg void OnViewUpfloor();          // 0x424700 — View→Up Floor (32954)
    afx_msg void OnViewDownfloor();        // 0x423ED0 — View→Down Floor (32955)
    afx_msg void OnTexturesShowinuse();    // 0x424B20 — Textures→Show In Use (32974)
    afx_msg void OnTexturesShowall();      // 0x42B440 — Textures→Show All / Ctrl-A (32973)

    // SELECTION TRANSFORMS — Clone + Flip/Rotate X/Y/Z (cores in select.cpp).
    afx_msg void OnSelectionClone();       // 0x425480 — Selection→Clone (33001)
    afx_msg void OnBrushFlipx();           // 0x4250A0 — Brush→Flip→X (32956)
    afx_msg void OnBrushFlipy();           // 0x4250C0 — Brush→Flip→Y (32957)
    afx_msg void OnBrushFlipz();           // 0x4250E0 — Brush→Flip→Z (32958)
    afx_msg void OnBrushRotatex();         // 0x425100 — Brush→Rotate→X (32959)
    afx_msg void OnBrushRotatey();         // 0x425190 — Brush→Rotate→Y (32960)
    afx_msg void OnBrushRotatez();         // 0x425220 — Brush→Rotate→Z (32961)

    // ── TOOLBAR command handlers (the icon bar, IDR_TOOLBAR152).  Each toggles a pref/
    //    mode and reflects it on its toolbar button (TB_CHECKBUTTON).  Bodies in
    //    mainfrm.cpp; ported verbatim from the CoD4Radiant CMainFrame handlers. ─────────
    afx_msg void OnTextureFlipX();             // 0x42BF40 — Brush_FlipTexture(0)        (33135)
    afx_msg void OnTextureFlipY();             // 0x42BF50 — Brush_FlipTexture(1)        (33136)
    afx_msg void OnTextureRotate90();          // 0x42BF60 — Brush_RotateTexture(90)     (33182)
    afx_msg void OnEditLayerCycle();           // 0x424010 — Material_SetMode cycle      (33238)
    afx_msg void OnToggleCameraMovementMode(); // 0x429EB0 — camera_mode 0/1/2 cycle     (32936)
    afx_msg void OnViewCubicclipping();        // 0x428F90 — m_bCubicClipping toggle     (32817)
    afx_msg void OnViewChange();               // 0x426400 — XY view-type cycle          (32781)
    afx_msg void OnSelectMouserotate();        // 0x428570 — free mouse-rotation toggle  (32810)
    afx_msg void OnSelectMousescale();         // 0x428D20 — mouse-scale mode toggle     (32813)
    afx_msg void OnScalelockX();               // 0x428BC0 — scale-lock X                (32814)
    afx_msg void OnScalelockY();               // 0x428B60 — scale-lock Y                (32815)
    afx_msg void OnScalelockZ();               // 0x428B90 — scale-lock Z                (32816)
    afx_msg void OnDontselectcurve();          // 0x429920 — m_bSelectCurves toggle      (32852)
    afx_msg void OnPatchWireframe();           // 0x42A300 — patch_wireframe 0/1/2 cycle (32857)
    afx_msg void OnPatchWeld();                // 0x42A400 — g_bPatchWeld toggle         (32858)
    afx_msg void OnPatchDrilldown();           // 0x42A510 — patch_drill_down toggle     (32865)
    afx_msg void ToggleLockPatchVertMode();    // 0x42B4F0 — bLockPatchVerts toggle      (33140)
    afx_msg void ToggleUnlockPatchVertMode();  // 0x42B510 — bUnlockPatchVerts toggle    (33139)
    afx_msg void OnCycleTerrainEdge();         // 0x42B530 — sel_cycle_edge_direction    (33141)
    afx_msg void OnToggleTextureAlphaRendering();// 0x429F10 — camera_masked toggle      (33138)
    afx_msg void OnDisableSelectionOfEntities();// 0x429F60 — entities_off toggle        (33142)
    afx_msg void OnDisableSelectionOfSky();    // 0x429FB0 — sky_brush_off toggle        (33169)
    afx_msg void OnToggleDrawSurfs();          // 0x42A040 — draw_toggle toggle          (33144)
    afx_msg void OnSelectableModels();         // 0x42A280 — m_bSelectableModels toggle  (33156)
    afx_msg void OnPlantModel();               // 0x42A0E0 — m_bDropModel toggle         (33159)
    afx_msg void OnForceZeroDropHeight();      // 0x42A000 — m_bForceZeroDropHeight      (33206)
    afx_msg void OnOrientToFloor();            // 0x4258F0 — m_bOrientModel toggle       (33195)
    afx_msg void OnTolerantWeld();             // 0x42A130 — m_bTolerantWeld toggle      (33155)
    afx_msg void OnVertSnapModel();            // 0x42A180 — m_bVertSnapModel toggle     (33207)
    afx_msg void OnVertSnapBrush();            // 0x42A1D0 — m_bVertSnapBrush toggle     (33208)
    afx_msg void OnVertSnapPrefab();           // 0x42A220 — m_bVertSnapPrefab toggle    (33209)
    afx_msg void OnPatchRedisperse();          // 0x42A990 — Patch_InsDelToggle          (32872)
    afx_msg void OnAdvancedEditDlg();          // 0x42BC90 — toggle terrain-paint dialog (33130)
    afx_msg void OnMiscCyclePreviewModels();   // 0x42BDD0 — w_cyclePreviewMode cycle    (35005)
    afx_msg void OnDropSelectedRelativeZ();    // 0x425940 — relative-Z drop of curve pts (35042)
    // Entity-display-mode popup (OnShowEntities pops MENU 160; its 6 items set
    // m_nEntityShowState).  Completes the toolbar — all 36 buttons now wired.
    afx_msg void OnShowEntities();                 // 0x42B300 — HandlePopup(160)        (32915)
    afx_msg void OnViewEntitiesasBoundingbox();    // 0x42B320 — m_nEntityShowState=0x1000  (32909)
    afx_msg void OnViewEntitiesasWireframe();      // 0x42B410 — 0x10001                  (32916)
    afx_msg void OnViewEntitiesasSelectedwireframe();// 0x42B380 — 0x101                  (32911)
    afx_msg void OnViewEntitiesasSelectedskinned();// 0x42B350 — 0x110                    (32912)
    afx_msg void OnViewEntitiesasSkinned();        // 0x42B3B0 — 0x10010                  (32913)
    afx_msg void OnViewEntitiesasSkinnedandboxed();// 0x42B3E0 — 0x11010                  (32914)
    void SetEntityCheck();                         // 0x42B1F0 — re-check the active mode's menu item

    // SMALL MODAL UTILITY DIALOGS (win_dlg.cpp — hand-built popups over ported cores).
    afx_msg void OnMiscFindbrush();          // 0x424B96 — Misc→Find brush (33023)
    afx_msg void OnMiscGoToPosition();       // 0x424BB9 — Misc→Go to position (33107)
    afx_msg void OnMiscMayaExport();         // Misc→Maya Export (33186) — ExportToMaya .mel generator
    afx_msg void OnSelectionArbitraryrotation();// 0x425300 — Selection→Rotate→Arbitrary (33033)
    afx_msg void OnHelpAbout();              // 0x4264F0 — Help→About (33038)
    afx_msg void OnErrorFile();              // 0x423B40 — File→Error file (36109)

    // TEXTURES→Render Method (Material_SetMode — now real; texwnd.cpp).
    afx_msg void OnRenderMethodMaterial();   // Textures→Render Method→Material (33232)
    afx_msg void OnRenderMethodLightmap();   // Textures→Render Method→Lightmap (33233)
    afx_msg void OnRenderMethodSmoothing();  // Textures→Render Method→Smoothing (36100)

    // TEXTURES→Render Method radio (32990..32994 → Texture_SetMode) — the binary's OTHER,
    // genuinely-dead-in-the-port render-method feature.  0x4243D0, ON_COMMAND_RANGE.
    afx_msg void OnRendermethodCaseTextures( UINT nID );

    // TEXTURES→Texture Filter submenu (33226..33230) — set the "r_textureMode" dvar.
    afx_msg void OnTextureFilterNearest();     // 0x424200 (33226)
    afx_msg void OnTextureFilterLinear();      // 0x424250 (33227)
    afx_msg void OnTextureFilterBilinear();    // 0x4242A0 (33228)
    afx_msg void OnTextureFilterTrilinear();   // 0x4242F0 (33229)
    afx_msg void OnTextureFilterAnisotropic(); // 0x424380 (33230)

    // PATCH menu — the U7 backfill of the five dead patch commands.
    afx_msg void OnCurvePatchdensetube();      // 0x42AB90 (32883) Primitives→Dense Cylinder
    afx_msg void OnCurvePatchverydensetube();  // 0x42AC40 (32884) Primitives→Very Dense Cylinder
    afx_msg void OnCurveCyclecap();            // 0x42B1A0 (32905) Cycle Cap Texture (Shift+Ctrl+N)
    afx_msg void OnAddTerrainRowColumn();      // 0x42B080 (33153) Insert→Add Terrain Row/Column

    // SELECTION → Scale... / Clipper → Split selected; PHYSICS → Cylinder / Box.
    afx_msg void OnSelectScale();              // 0x4283D0 (32809) Selection→Scale...
    afx_msg void OnSplitSelected();            // 0x4271D0 (32794) Clipper→Split selected
    afx_msg void OnMakePhysCylinder();         // 0x4291D0 (36113) Physics→Cylinder
    afx_msg void OnMakePhysBox();              // 0x429200 (36120) Physics→Box

    // ── U7 item 8: the remaining dead CMainFrame singles from SWEEP_C's table. ──
    afx_msg void OnDeleteExportables();        // 0x424E30 (206)   Misc→Delete exportables
    afx_msg void OnHelpCommandlist();          // 0x426E00 (32790) Help→Command list...
    afx_msg void OnLinkKeepSelection();        // 0x423EE0 (1085)  Shift+O LinkSelectionToggle
    afx_msg void ToggleCamera();               // 0x423A50 (32776) m_bCamPreview flip
    afx_msg void OnPopupRenderMethod();        // 0x4263E0 (32779) HandlePopup(124)
    afx_msg void OnPopupSelection();           // 0x4263F0 (32780) HandlePopup(125)
    afx_msg void OnViewCameraupdate();         // 0x426450 (32782) View→Camera update
    afx_msg void OnCurvePatchinvertedendcap(); // 0x42A500 (32863) EMPTY in the binary
    afx_msg void OnCurvePatchinvertedbevel();  // 0x42A4F0 (32864) EMPTY in the binary
    afx_msg void OnCurveInsertrowSingle();     // 0x42A5B0 (32867) Ctrl+Num+
    afx_msg void OnCurveInsertcolumnSingle();  // 0x42A560 (32868) Shift+Ctrl+Num+
    afx_msg void OnCurveDeleterowSingle();     // 0x42A650 (32869) Ctrl+Num-
    afx_msg void OnCurveDeletecolumnSingle();  // 0x42A600 (32870) Shift+Ctrl+Num-
    afx_msg void OnPatchBend();                // 0x42A950 (32882) bend-mode toggle
    afx_msg void OnHideUnselected2();          // 0x42B6B0 (32925) Shift+Alt+Ctrl+H
    afx_msg void OnMiscBenchmark();            // 0x424B70 (32978) EMPTY in the binary
    afx_msg void OnPatchTab();                 // 0x42A9E0 (33089) Tab
    afx_msg void OnPatchEnter();               // 0x42A9D0 (33090) EMPTY in the binary
    afx_msg void OnGotoPos_unk();              // 0x42AE90 (33091) Ctrl+G
    afx_msg void OnVertexSelectUp();           // 0x426880 (33165) Ctrl+Up
    afx_msg void OnVertexSelectDown();         // 0x4268A0 (33166) Ctrl+Down
    afx_msg void OnVertexSelectRight();        // 0x4268C0 (33167)
    afx_msg void OnVertexSelectLeft();         // 0x4268E0 (33168)
    afx_msg void OnRedistPatchPoints();        // 0x42A270 (33170) Shift+F
    afx_msg void OnTurnTerrainEdges();         // 0x4294E0 (33179) Alt+F2
    afx_msg void OnDropPatchVertices();        // 0x42AE00 (33213) Shift+Alt+Ctrl+D
    afx_msg void OnSelectTargettedEntity();    // 0x425560 (36110) Ctrl+E
    afx_msg void OnFileClose();                // 0x423A70 (57602) EMPTY in the binary
    afx_msg void OnFilePrint();                // 0x423B60 (57607) EMPTY in the binary
    afx_msg void OnFilePrintPreview();         // 0x423B70 (57609) EMPTY in the binary

    // Layered-material authoring window (layeredmaterialwnd.cpp).
    afx_msg void OnToggleLayeredMaterials(); // 0x42BFE0 — Window→Layered Materials (F4, 35008)
    afx_msg void OnSaveLayeredMaterials();   // 0x42C020 — File→Save Layered Materials (35009)

    // PATCH menu — create a curve patch from a brush (Patch_BrushToMesh) + grow/shrink
    // the control grid (Patch_AdjustSelected).  Cores in pmesh.cpp (data layer).
    afx_msg void OnCurveSimplepatchmesh();   // 0x429A20 — Curve→Simple Patch Mesh (32856)
    afx_msg void OnCurveSimpleterrainpatch(); // Curve→Simple Terrain Patch (32939)
    afx_msg void OnCurvePatchtube();         // 0x42A3B0 — Patch→Primitives→Cylinder (32859)
    afx_msg void OnCurvePatchcone();         // 0x42A360 — Patch→Primitives→Cone (32860)
    afx_msg void OnCurvePatchendcap();       // 0x42A4A0 — Patch→Primitives→End Cap (32861)
    afx_msg void OnCurvePatchbevel();        // 0x42A450 — Patch→Primitives→Bevel (32862)
    afx_msg void OnCurvePatchsquare();       // 0x42AF00 — Patch→Primitives→Square Cylinder (32891)
    afx_msg void OnCurveSquareBevel();       // 0x42B590 — Patch→Primitives→Square Bevel (32920)
    afx_msg void OnCurveSquareEndcap();      // 0x42B5E0 — Patch→Primitives→Square End Cap (32921)
    afx_msg void OnCurveInsertcolumn();      // 0x42A560 — Patch→Insert→Insert (2) Columns (32874)
    afx_msg void OnCurveInsertAddcolumn();   // 0x42A6A0 — Patch→Insert→Add (2) Columns (32873)
    afx_msg void OnCurveInsertrow();         // 0x42A5B0 — Patch→Insert→Insert (2) Rows (32876)
    afx_msg void OnCurveInsertAddrow();      // 0x42A5B0 — Patch→Insert→Add (2) Rows (32875)
    afx_msg void OnCurveDeleteFirstcolumn(); // 0x42A810 — Patch→Delete→First (2) Columns (32877)
    afx_msg void OnCurveDeleteLastcolumn();  // 0x42A8B0 — Patch→Delete→Last (2) Columns (32878)
    afx_msg void OnCurveDeleteFirstrow();    // 0x42A860 — Patch→Delete→First (2) Rows (32879)
    afx_msg void OnCurveDeleteLastrow();     // 0x42A900 — Patch→Delete→Last (2) Rows (32880)
    afx_msg void OnCurveNegative();          // 0x42A7E0 — Patch→Negative (vertical flip) (32881)
    afx_msg void OnCurveMatrixTranspose();   // 0x42B1E0 — Patch→Matrix→Transpose (32906)
    afx_msg void OnPatchNaturalize();        // 0x42AE10 — Patch→Naturalize (32890)
    afx_msg void OnSelectionInvert();        // 0x42B6F0 — Selection→Invert (Ctrl+I, 33101)
    afx_msg void OnCurveRedisperseCols();    // 0x42AD80 — Patch→Redisperse→Columns (32889)
    afx_msg void OnCurveRedisperseRows();    // 0x42AD90 — Patch→Redisperse→Rows (32888)
    afx_msg void OnCurveNegativeTextureX();  // 0x42A7F0 — Patch→Negative Texture X (32899)
    afx_msg void OnCurveNegativeTextureY();  // 0x42A800 — Patch→Negative Texture Y (32903)

    // PATCH/CURVE OPERATION CLUSTER (Cap / Thicken / Weld / Split) + TERRAIN row/col.
    afx_msg void OnCurveCap();               // 0x42AD40 — Curve→Cap (32885)
    afx_msg void OnPatchCap();               // 0x42AE50 — Patch→Cap naturalize (35040)
    afx_msg void OnCurveThicken();           // 0x42B0D0 — Curve→Thicken (32904, CDialogThick)
    afx_msg void OnSplitPatch();             // 0x42B0C0 — Curve→Split (33158)
    afx_msg void ExtrudeTerrainRow2();       // 0x42B0A0 — Terrain→Extrude Row/Col (33192)
    afx_msg void OnRemoveTerrainRowColumn(); // 0x42B0B0 — Terrain→Remove Row/Col (33154)
    afx_msg void OnCurveToTerrain();         // 0x429B30 — Curve→Terrain (35041)
    afx_msg void OnFaceToTerrain();          // 0x429BE0 — Face→Terrain (36102)
    afx_msg void OnSelectionConnect();       // 0x425510 — Selection→Connect (33021)

    // VIEW→SHOW overlay toggles (d_xyShowFlags; IDB 0x42ba40-0x42bc20).  Each flips one
    // visibility bit, re-checks its menu item, pushes the state into the filter pane's
    // checkboxes, and repaints.  The binary returns LRESULT/HWND; MFC ON_COMMAND handlers
    // are void, so these wrap the binary body and drop the (unused) return.
    afx_msg void OnSelectNames();           // 0x42ba40 — View→Show→Names (33971)
    afx_msg void OnSelectAngles();          // 0x42bac0 — View→Show→Angles (33972, DrawAngles)
    afx_msg void OnSelectBlocks();          // 0x42bb00 — View→Show→Blocks (33973, XY_DrawBlockGrid)
    afx_msg void OnSelectCoordinates();     // 0x42bb60 — View→Show→Coordinates (33975)
    afx_msg void OnSelectReverseFilter();   // 0x42bc20 — View→Show→Reverse Filter (36127)
    afx_msg void OnSelectConnections();     // 0x42bbc0 — View→Show→Connections (33974)
    // ALL SIX View→Show toggles are now wired (Angles + Blocks completed this unit) — each
    // flips its d_xyShowFlags bit, re-checks its menu item, and its draw consumer reads the
    // bit live: Names(0x8)/Angles(0x2)/Blocks(0x10)/Connections(0x4)/Coordinates(0x20)/
    // ReverseFilter(0x40).

    // SCRIPT-GROUP / FIXED-SIZE LIGHT keyboard commands (cores in brush.cpp + pmesh.cpp).
    // Accelerators are already in g_radiantCommands; these bind the WM_COMMAND ids to the
    // thin CMainFrame handlers the binary jumps through (0x428E10-0x428EE0).
    afx_msg void OnDisassociateEntities();  // 0x428E10 — DisassociateEntities                       (33151)
    afx_msg void OnSelectedAssociated();    // 0x428E20 — SelectedAssociated                         (33152)
    // Script-Group COLOUR commands (cores in scriptgroup.cpp). OnScriptGroup_01 / _Disassociate
    // are the Script-Group dialog's "Add Color"/"Disassociate" control ids (9/10) routed to the
    // frame (the dialog proc dispatches them too); both force the colour-team key first
    // (ScriptGroup_SyncGroupKeyToTeam). OnScriptGroup (menu 200) and OnAssociateEntities (accel
    // 33150) are distinct thunks that open the dialog via AssociateEntities.
    afx_msg void OnScriptGroup_01();          // 0x4264A0 — SyncGroupKeyToTeam + AddColorToSelection  (9)
    afx_msg void OnScriptGroup_Disassociate();// 0x426460 — SyncGroupKeyToTeam + TriggerNumber        (10)
    afx_msg void OnScriptGroup();             // 0x424E20 — "Script group" menu → AssociateEntities   (200)
    afx_msg void OnAssociateEntities();       // 0x428E00 — Associate Entities accel → AssociateEntities (33150)
    afx_msg void OnLightShiftUp();          // 0x428E30 — Light_01(1.1)                               (33145)
    afx_msg void OnLightShiftDown();        // 0x428E50 — Light_01(0.9)                               (33146)
    afx_msg void OnCyclinderHeightUp();     // 0x428E70 — Light_Height(1.1)                           (33176)
    afx_msg void OnCyclinderHeightDown();   // 0x428E90 — Light_Height(0.9)                           (33177)
    afx_msg void OnCameraUp();              // 0x426900 — camera.origin[2]+=32 (raise, 'D')           (33055)
    afx_msg void OnCameraDown();            // 0x426680 — camera.origin[2]-=32 (lower, 'C')           (33056)
    afx_msg void OnCameraAngleUp();         // 0x4265d0 — camera.angles[0]+=22.5 clamp 85 (pitch,'A') (33061)
    afx_msg void OnCameraAngleDown();       // 0x426590 — camera.angles[0]-=22.5 clamp -85 (pitch,'Z')(33062)
    afx_msg void OnOverBrightShiftUp();     // 0x428EB0 — OverbrightShift(-0.05)+Patch_Subdivide(-1)  (33147)
    afx_msg void OnOverBrightShiftDown();   // 0x428EE0 — OverbrightShift(0.05)+Patch_Subdivide(1)    (33148)

    // ── ON_UPDATE_COMMAND_UI handlers (the binary's 4). ──
    afx_msg void OnUpdateViewCameraupdate( CCmdUI *pCmdUI ); // 0x4264B0 (32782)
    afx_msg void OnUpdateEditUndo( CCmdUI *pCmdUI );         // 0x428750 (57643)
    afx_msg void OnUpdateEditRedo( CCmdUI *pCmdUI );         // 0x428790 (57644)
    afx_msg void OnUpdateFileSaveregion( CCmdUI *pCmdUI );   // 0x429030 (32827)
    bool m_bCamPreview = false;             // CMainFrame member (0x4264B0 reads it): camera-update preview state

    // ── File menu ──
    afx_msg void OnFileImportmap();         // 0x429290 (32844) — Load (merge .map)
    afx_msg void OnFileExportmap();         // 0x4293A0 (32845) — Save Selected
    afx_msg void OnFileSaveregion();        // 0x429020 (32827) — Save Region

    // ── Light-preview submenu (F8) ──
    afx_msg void OnEnableLightPreview();    // 0x4240C0 (33950)
    afx_msg void OnPreviewSun();            // 0x424060 (36108)
    afx_msg void OnStartPreviewSelected();  // 0x424120 (33951)
    afx_msg void OnStopPreviewSelected();   // 0x424170 (33952)
    afx_msg void OnClearPreviewList();      // 0x4241C0 (33953)
    afx_msg void OnPreviewAtMaxIntensity(); // 0x425670 (36122)

    // ── Colors menu — each pops DoColor(paletteIndex) then flags a full window update. ──
    // (EAs / palette indices decoded from the CMainFrame AFX_MSGMAP + each thunk body.)
    afx_msg void OnTextureBackground();          // 0x424E40 (33013) DoColor(0)  Texture Background
    afx_msg void OnColorsXyBackground();         // 0x424EC0 (33014) DoColor(1)  Grid Background
    afx_msg void OnColorsMinor();                // 0x424EA0 (33020) DoColor(2)  Grid Minor
    afx_msg void OnColorsMajor();                // 0x424E80 (33019) DoColor(3)  Grid Major
    afx_msg void OnColorsCameraBack();           // 0x424E60 (209)   DoColor(4)  Camera Background
    afx_msg void OnColorsGridblock();            // 0x427320 (32803) DoColor(7)  Grid Block
    afx_msg void OnColorsGridText();             // 0x4272C0 (32799) DoColor(8)  Grid Text
    afx_msg void OnColorsBrush();                // 0x427240 (32800) DoColor(9)  Default Brush
    afx_msg void OnColorsSelectedbrush();        // 0x4272E0 (32801) DoColor(10) Selected Brush
    afx_msg void OnColorsSelectedbrushCamera();  // 0x427300 (33171) DoColor(11) Selected Brush Camera Tint
    afx_msg void OnColorsClipper();              // 0x427280 (32802) DoColor(12) Clipper
    afx_msg void OnColorsViewname();             // 0x427340 (32804) DoColor(13) Active View Name
    afx_msg void OnColorsDetailBrush();          // 0x427360 (33131) DoColor(14) Detail Brush
    afx_msg void OnColorsToggleDrawSurfs();      // 0x427380 (33149) DoColor(15) Draw toggle surfs
    afx_msg void OnColorsSelfaceCamera();        // 0x4273E0 (33137) DoColor(16) Selected Face Camera Tint
    afx_msg void OnColorsFuncGroup();            // 0x427400 (189)   DoColor(17) Func Group
    afx_msg void OnColorsFuncCullGroup();        // 0x427420 (211)   DoColor(18) Func Cullgroup
    afx_msg void OnColorsWeaponclip();           // 0x4273A0 (195)   DoColor(19) Weapon Clip Brush
    afx_msg void OnColorsSizeInfo();             // 0x427440 (212)   DoColor(20) Size Info
    afx_msg void OnColorsModel();                // 0x427460 (213)   DoColor(21) Model
    afx_msg void OnColorsUnknown208();           // 0x4273C0 (208)   DoColor(22) (no menu item)
    afx_msg void OnColorsWireframe();            // 0x4272A0 (33194) DoColor(23) Wireframe
    afx_msg void OnColorsFrozenLayers();         // 0x427260 (35020) DoColor(24) Frozen Layers
    afx_msg void OnMiscSelectentitycolor();      // 0x424C10 (33036 / K accel) pick a light's _color

    // ── Colors→Themes — write a whole preset palette into d_savedinfo.colors[0..26]. ──
    afx_msg void OnThemeQ4();          // 0x427480 (32805) QE4 Original
    afx_msg void OnThemeQ3();          // 0x427760 (32806) Q3Radiant Original
    afx_msg void OnThemeBlackGreen();  // 0x427A40 (32807) Black and Green
    afx_msg void OnThemeInverted();    // 0x427D30 (33143) Inverted
    afx_msg void OnThemeGrey();        // 0x428040 (210)   Gray
    afx_msg void OnDynamicLighting();  // 0x429960 (32854) spawn a floating popup CCamWnd

    // Frame-wide keyboard — TranslateAccelerator (IDR_MAIN_ACCEL) then the
    // command-map Ctrl+Z/Y undo/redo, handled here so they work regardless of which
    // child view has focus.
    virtual BOOL PreTranslateMessage( MSG *pMsg );

    HACCEL m_hAccel = NULL;               // IDR_MAIN_ACCEL

    // QE window-layout style (IDB g_pParentWnd+0x7D4).  0=floating, 1/2=docked QE4
    // variants (the integrated inspector with all 4 tabs), 3=4-pane.  This port uses a
    // fixed integrated layout → style 1.  SetInspectorMode reads it (Texture/Console
    // switching is a no-op in styles 0/3 where those are separate windows).
    int m_nCurrentStyle = 1;

    // INSPECTOR TAB SWITCHING — the inspector mode-switch menu commands (SetInspectorMode).
    // Each toggles the inspector to that mode (or hides it if already there + visible),
    // faithful to the IDB handlers.  Console mode has no menu command in the binary (it is
    // reached via the tab control); the docked console is toggled separately (OnToggleconsole).
    afx_msg void OnViewEntity();          // 0x423f00 — View→Toggle→Entity View (33017)
    afx_msg void OnViewTextureMode();     // 0x424440 — Textures→Texture inspector tab (33018)
    afx_msg void OnViewConsole();         // 0x423e10 — View→Toggle→Console View (33016)
    afx_msg void OnFilterDlg();           // 0x42b7a0 — View→Filter Settings (33104)

    // Textures-menu texture-filter submenus (Usage/Locale/Surface type), built at startup
    // by FillTextureMenu.  ON_COMMAND_RANGE handlers: nID - base = the filter index.
    afx_msg void OnFilterUsage( UINT nID );       // 0x4243e0 — ON_COMMAND_RANGE 60000..60255
    afx_msg void OnFilterLocale( UINT nID );      // 0x424400 — ON_COMMAND_RANGE 60256..60511
    afx_msg void OnFilterSurfaceType( UINT nID ); // 0x424420 — ON_COMMAND_RANGE 60512..60767

protected:
    // The QE4 layout — five render windows + status bar + menu/accel, then
    // R_BeginRegistrationInternal (the renderer bring-up).
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnTimer( UINT_PTR nIDEvent );
    // Splitter resize bars on the pane gutters (the QE4 CSplitterWnd tracker equivalent).
    void         RelayoutPanes();         // lay panes out from the current split fractions
    int          HitTestBar( int x, int y );  // which gutter (0 = none) is at client (x,y)
    afx_msg BOOL OnSetCursor( CWnd *pWnd, UINT nHitTest, UINT message );
    afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnMouseMove( UINT nFlags, CPoint point );
    DECLARE_MESSAGE_MAP()
};

// Status-bar sink (the real MainFrm_SetStatusText; replaces the engine_stubs no-op).
void MainFrm_SetStatusText( int pane, const char *text );
