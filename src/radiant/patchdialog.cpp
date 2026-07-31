#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Patch creation and advanced terrain-paint dialogs.

#include "stdafx.h"
#include "res/resource.h"
#include "qe3.h"
#include <stdlib.h>

// ═══════════════════════════════════════════════════════════════════════════════
//  CAdvPatchEditDlg — the terrain-paint settings dialog (paint epic piece 6b).
//  A MODELESS CDialog over IDD_ADVPATCHEDIT (the controls carry the binary's exact
//  IDs).  The 3 radius/strength EDITs sync into the apply chain's control table
//  (g_curveEditCtrls via CurveEdit_SetCtrl) on change; the mode RADIOs and channel
//  CHECKBOXes are read LIVE by sub_401DB0 / sub_43E6F0 (piece 7) through GetDlgItem
//  on the global dialog pointer (AdvPatchEdit_GetDlg).  Modeless so it stays open
//  while the user paints; Close/Cancel just hides it.
// ═══════════════════════════════════════════════════════════════════════════════
extern void CurveEdit_SetCtrl( int slot, int id, float value );   // pmesh.cpp (gate seeder)
extern void  CurveEdit_BindData( int slot, int trackbarId, int editId, float defVal, float mn, float mx, float step );
extern float CurveEdit_SnapStore( int slot, float value );        // sub_4010D0 clamp+snap
extern int   CurveEdit_TrackbarId( int slot );
extern int   CurveEdit_EditId( int slot );
extern int   CurveEdit_StepCount( int slot );
extern int   CurveEdit_StepIndex( int slot );
extern void  CurveEdit_SetHwnds( int slot, void *hTrackbar, void *hEdit );
extern float CurveEdit_SetFromStep( int slot, int pos );
extern float CurveEdit_DisplayValue( int slot );
extern float CurveEdit_InputToValue( int slot, float typed );
extern void  CurveEdit_SetRange( int slot, float maxVal );        // sub_4015C0 range edits

// Trackbar messages (raw, to avoid an afxcmn/CSliderCtrl dependency in this TU).
#define KRAD_TBM_GETPOS    0x0400   // WM_USER+0
#define KRAD_TBM_SETPOS    0x0405   // WM_USER+5
#define KRAD_TBM_SETRANGE  0x0406   // WM_USER+6
extern unsigned int  g_paintColorBGR;   // paint target colour {B,G,R} (defined below)
extern unsigned char g_paintColorA;     // paint target alpha       (defined below)

class CAdvPatchEditDlg : public CDialog
{
public:
    CAdvPatchEditDlg( CWnd *parent ) : CDialog( IDD_ADVPATCHEDIT, parent ) {}

protected:
    // Push a slot's stored value out to its slider thumb + buddy-edit text (sub_401000 + TBM_SETPOS).
    void RefreshSlot( int slot )
    {
        HWND hTb = ::GetDlgItem( m_hWnd, CurveEdit_TrackbarId( slot ) );
        if ( hTb ) ::SendMessageA( hTb, KRAD_TBM_SETPOS, TRUE, CurveEdit_StepIndex( slot ) );
        char s[64];
        sprintf( s, "%g", CurveEdit_DisplayValue( slot ) );
        SetDlgItemText( CurveEdit_EditId( slot ), s );
    }
    // Buddy edit changed: read text -> value (sub_401CF0 transform for amplitude) -> clamp+snap -> refresh.
    void SyncCtrl( int slot )
    {
        CString s;
        GetDlgItemText( CurveEdit_EditId( slot ), s );
        CurveEdit_SnapStore( slot, CurveEdit_InputToValue( slot, (float)atof( (LPCSTR)s ) ) );
        RefreshSlot( slot );
    }

    virtual BOOL OnInitDialog()
    {
        CDialog::OnInitDialog();
        // Bind the 3 param slots with the binary's baked ids / defaults / ranges / steps
        // (trackbar 1424/1425/1426, buddy edit 1428/1429/1430), then size the sliders + show values.
        CurveEdit_BindData( 0, 1424, 1428, 16.0f, 0.0f, 1024.0f, 16.0f );  // inner radius
        CurveEdit_BindData( 1, 1425, 1429, 64.0f, 0.0f, 1024.0f, 16.0f );  // outer radius
        CurveEdit_BindData( 2, 1426, 1430,  8.0f, 0.0f,   16.0f,  1.0f );  // amplitude exponent
        for ( int slot = 0; slot < 3; ++slot )
        {
            HWND hTb = ::GetDlgItem( m_hWnd, CurveEdit_TrackbarId( slot ) );
            HWND hEd = ::GetDlgItem( m_hWnd, CurveEdit_EditId( slot ) );
            if ( hTb ) ::SendMessageA( hTb, KRAD_TBM_SETRANGE, TRUE, (LPARAM)( CurveEdit_StepCount( slot ) << 16 ) );
            CurveEdit_SetHwnds( slot, hTb, hEd );
            RefreshSlot( slot );
        }
        SetDlgItemInt( 1482, 1024 );  SetDlgItemInt( 1483, 1024 );   // inner/outer slider max (range edits)
        CheckDlgButton( 1435, BST_CHECKED );   // default mode: Paint Height (sub_401DB0 -> 0)
        CheckDlgButton( 1469, BST_CHECKED );   // default channel: Height (mask & 1)
        return TRUE;
    }

    afx_msg void OnEditInner()    { SyncCtrl( 0 ); }
    afx_msg void OnEditOuter()    { SyncCtrl( 1 ); }
    afx_msg void OnEditStrength() { SyncCtrl( 2 ); }
    // Slider-range edits (1482/1483, IDC_SURF_DLG_TBOX_INNER/OUTER_RAD_2): set the inner/outer
    // slider's max, rebuild its [0,64] range and re-snap the value (sub_4015C0 flt_73C6B0/D0).
    void SyncRange( int slot, int rangeEditId )
    {
        CString s;
        GetDlgItemText( rangeEditId, s );
        float mx = (float)atof( (LPCSTR)s );
        if ( mx == 0.0f ) { mx = 1024.0f; SetDlgItemText( rangeEditId, "1024" ); }
        CurveEdit_SetRange( slot, mx );
        HWND hTb = ::GetDlgItem( m_hWnd, CurveEdit_TrackbarId( slot ) );
        if ( hTb ) ::SendMessageA( hTb, KRAD_TBM_SETRANGE, TRUE, (LPARAM)( CurveEdit_StepCount( slot ) << 16 ) );
        RefreshSlot( slot );
    }
    afx_msg void OnRangeInner() { SyncRange( 0, 1482 ); }
    afx_msg void OnRangeOuter() { SyncRange( 1, 1483 ); }
    // Slider dragged (WM_HSCROLL): map thumb pos -> value, refresh the buddy edit.
    afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
    {
        if ( pScrollBar )
        {
            int id = pScrollBar->GetDlgCtrlID();
            for ( int slot = 0; slot < 3; ++slot )
                if ( CurveEdit_TrackbarId( slot ) == id )
                {
                    int pos = (int)::SendMessageA( pScrollBar->m_hWnd, KRAD_TBM_GETPOS, 0, 0 );
                    CurveEdit_SetFromStep( slot, pos );
                    RefreshSlot( slot );
                    return;
                }
        }
        CDialog::OnHScroll( nSBCode, nPos, pScrollBar );
    }
    // sub_4021E0 — "Color..." button (real id 1460).  Seeds CColorDialog with the current
    // paint colour and on OK stores the raw COLORREF verbatim (this[323] = m_cc.rgbResult),
    // then redraws the owner-draw swatch.  NOTE: the binary stores the raw COLORREF, so we
    // do NOT re-pack channel order here (the eyedropper writes a different byte order — that
    // is a pre-existing inconsistency in the original, preserved for fidelity).
    afx_msg void OnSetColor()
    {
        CColorDialog dlg( (COLORREF)g_paintColorBGR, 0, this );
        if ( dlg.DoModal() == IDOK )
            g_paintColorBGR = dlg.GetColor();          // raw COLORREF, matching this[323]
    }
    // sub_4022A0 — "Alpha..." button (real id 1464).  Seeds the dialog with the current alpha
    // as a grey, and on OK sets alpha = (R+G+B)/3 of the chosen colour.
    afx_msg void OnSetAlpha()
    {
        unsigned int a = g_paintColorA;
        CColorDialog dlg( (COLORREF)( a | ( ( a | ( a << 8 ) ) << 8 ) ), 0, this );
        if ( dlg.DoModal() == IDOK )
        {
            COLORREF c = dlg.GetColor();
            g_paintColorA = (unsigned char)( ( ( c & 0xFF ) + ( ( c >> 16 ) & 0xFF ) + ( ( c >> 8 ) & 0xFF ) ) / 3 );
        }
    }
    // sub_402490 — CurvEditDlg::OnColorAlpha (WM_DRAWITEM): paint the two owner-draw swatches.
    afx_msg void OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT dis )
    {
        if ( nIDCtl == 1462 )                                  // IDC_SURF_DLG_COLOR_OWNER
        {
            HBRUSH b = CreateSolidBrush( (COLORREF)g_paintColorBGR );
            FillRect( dis->hDC, &dis->rcItem, b );
            DeleteObject( b );
        }
        else if ( nIDCtl == 1466 )                             // IDC_SURF_DLG_ALPA_OWNER
        {
            unsigned int a = g_paintColorA;
            HBRUSH b = CreateSolidBrush( (COLORREF)( a | ( ( a | ( a << 8 ) ) << 8 ) ) );
            FillRect( dis->hDC, &dis->rcItem, b );
            DeleteObject( b );
        }
        else
            CDialog::OnDrawItem( nIDCtl, dis );
    }
public:
    // sub_4023D0 — CurvEditDlg::OnHeightText: show the picked height in the 1467 edit box.
    void OnHeightText( float h )
    {
        char s[64];
        sprintf( s, "%.0f", h );
        SetDlgItemText( 1467, s );
    }
protected:

    virtual void OnCancel()       { ShowWindow( SW_HIDE ); }   // modeless: hide, don't EndDialog
    afx_msg void OnClose()        { ShowWindow( SW_HIDE ); }

    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP( CAdvPatchEditDlg, CDialog )
    ON_EN_KILLFOCUS( 1428, OnEditInner )    // buddy edits (sliders are 1424/1425/1426)
    ON_EN_KILLFOCUS( 1429, OnEditOuter )
    ON_EN_KILLFOCUS( 1430, OnEditStrength )
    ON_EN_KILLFOCUS( 1482, OnRangeInner )   // inner/outer slider max range
    ON_EN_KILLFOCUS( 1483, OnRangeOuter )
    ON_WM_HSCROLL()                         // slider drags -> param values
    ON_BN_CLICKED( 1460, OnSetColor )       // sub_4021E0  "Color..."
    ON_BN_CLICKED( 1464, OnSetAlpha )       // sub_4022A0  "Alpha..."
    ON_WM_DRAWITEM()                        // sub_402490  owner-draw swatches
    ON_WM_CLOSE()
END_MESSAGE_MAP()

static CAdvPatchEditDlg *g_pAdvPatchDlg = nullptr;

// Accessor used by sub_401DB0 / sub_43E6F0 (piece 7) — returns the live dialog (or null,
// in which case those fall back to their not-found defaults).  Mirrors the binary's
// global AdvPatchEditDlg (0x25D6098).
CWnd *AdvPatchEdit_GetDlg() { return g_pAdvPatchDlg; }

// Forwarder so the free paint-drag (sub_43E6F0) can push the eyedroppered height into the
// dialog's 1467 edit box (CurvEditDlg::OnHeightText) without befriending the class.
static void AdvPatchEdit_ShowHeight( float h )
{
    if ( g_pAdvPatchDlg && g_pAdvPatchDlg->GetSafeHwnd() )
        g_pAdvPatchDlg->OnHeightText( h );
}

// CurvEditDlg::OnSomeSetting (0x401F90) — is the "apply to unselected patches too" checkbox
// (1432, IDB dword_25D6570) checked on the live dialog?  Gates paint/soft-sel onto the active
// (unselected) brush list (sub_43E4F0 / sub_43DD00) and the no-selection paint start (Drag_Begin).
int CurvEditDlg_OnSomeSetting()
{
    return ( g_pAdvPatchDlg && g_pAdvPatchDlg->GetSafeHwnd()
             && ::IsDlgButtonChecked( g_pAdvPatchDlg->m_hWnd, 1432 ) ) ? 1 : 0;
}

// sub_401E10 — programmatically select the mode radio whose value == `mode` (walks the
// mode table, BM_SETCHECKs each), then repaint.  Used by Patch_FinishCurveDrag to switch
// from "Grab Value" (5) to "Flatten" (2) after an eyedropper drag.
void AdvPatchEdit_SetMode( int mode )
{
    static const struct { int id, val; } modes[7] =
        { {1435,0},{1439,1},{1437,2},{1436,3},{1438,4},{1468,5},{1440,6} };
    if ( !g_pAdvPatchDlg || !g_pAdvPatchDlg->GetSafeHwnd() )
        return;
    for ( int i = 0; i < 7; ++i )
        g_pAdvPatchDlg->CheckDlgButton( modes[i].id, modes[i].val == mode ? BST_CHECKED : BST_UNCHECKED );
    g_pAdvPatchDlg->InvalidateRect( nullptr, FALSE );
}

// Show (creating on first use) the modeless terrain-paint settings dialog.
void AdvPatchEdit_Show( CWnd *parent )
{
    if ( !g_pAdvPatchDlg )
    {
        g_pAdvPatchDlg = new CAdvPatchEditDlg( parent );
        g_pAdvPatchDlg->Create( IDD_ADVPATCHEDIT, parent );
    }
    g_pAdvPatchDlg->ShowWindow( SW_SHOW );
}

// CMainFrame::OnAdvancedEditDlg (0x42BC90, cmd 33130) toggle: hide if visible, else show.
void AdvPatchEdit_Toggle( CWnd *parent )
{
    if ( g_pAdvPatchDlg && g_pAdvPatchDlg->GetSafeHwnd() && g_pAdvPatchDlg->IsWindowVisible() )
        g_pAdvPatchDlg->ShowWindow( SW_HIDE );
    else
        AdvPatchEdit_Show( parent );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  ShowInfoDialog (0x40BE90) — the modeless "Information" state prompt.
//
//  A caption-only popup holding one disabled multiline edit (IDD_INFORMATION, PE
//  resource 150).  Patch BEND and INSERT/DELETE mode use it to tell the user which
//  key does what for the current sub-state; the message pointer is one of the
//  g_pBendStateMsg / g_pInsDelStateMsg literals below.
//
//  The binary keeps the dialog in a static object (off_25D5BF0) and tests its m_hWnd
//  (dword_25D5C10 == object+0x20) to decide whether to Create it; this port uses a
//  lazily-new'd pointer, the same idiom as g_pAdvPatchDlg above.  SetFocus back to the
//  main frame is faithful (0x40bec7) — the prompt must never steal keyboard focus, or
//  the TAB/ENTER/ESC keys the message describes would go to the prompt instead of the
//  view that implements the mode.
// ═══════════════════════════════════════════════════════════════════════════════

// Bend-state prompts, indexed by g_nPatchBendState + 1 (IDB g_pBendStateMsg 0x73B114).
const char *g_pBendStateMsg[4] =
{
    "Use TAB to cycle through available bend axis. Press ENTER when the desired one is highlighted.",
    "Use TAB to cycle through available rotation axis. This will LOCK around that point. You may also use Shift + Middle Click to select an arbitrary point. Press ENTER when the desired one is highlighted",
    "Use TAB to choose which side to bend. Press ENTER when the desired one is highlighted.",
    "Use the MOUSE to bend the patch. It uses the same ui rules as Free Rotation. Press ENTER to accept the bend, press ESC to abandon it and exit Bend mode",
};

// Insert/delete (redisperse) prompt (IDB off_73B128[0] 0x73B128).
const char *g_pInsDelStateMsg = "Use TAB to cycle through available rows/columns for insertion/deletion. Press INS to insert at the highlight, DEL to remove the pair";

static CDialog *g_pInfoDlg = nullptr;   // IDB off_25D5BF0

void ShowInfoDialog( const char *msg )
{
    if ( !g_pInfoDlg || !g_pInfoDlg->GetSafeHwnd() )       // IDB: if ( !dword_25D5C10 )
    {
        if ( !g_pInfoDlg )
            g_pInfoDlg = new CDialog();
        g_pInfoDlg->Create( IDD_INFORMATION, AfxGetMainWnd() );
    }
    if ( !g_pInfoDlg->GetSafeHwnd() )
        return;                                            // PORT guard: headless / no resources
    g_pInfoDlg->SetDlgItemTextA( IDC_INFO_TEXT, msg );     // IDB off_25D5C64 = the edit member
    g_pInfoDlg->ShowWindow( SW_SHOW );
    // 0x40bec7: SetFocus(g_pParentWnd). g_pParentWnd IS the main frame, and mainfrm.h is
    // only included further down this TU, so go through AfxGetMainWnd() (same window).
    if ( CWnd *mainWnd = AfxGetMainWnd() )
        mainWnd->SetFocus();
}

// The hide half (CWnd::ShowWindow(&off_25D5BF0, SW_HIDE), guarded by dword_25D5C10).
void HideInfoDialog()
{
    if ( g_pInfoDlg && g_pInfoDlg->GetSafeHwnd() )
        g_pInfoDlg->ShowWindow( SW_HIDE );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_401DB0 (mode selector) + sub_43E6F0 (the addpoint PAINT DRAG) — paint epic 7.
//  This is the bridge from a mouse drag (Drag_MouseMoved sel_addpoint+Alt) to the apply
//  chain: pick the terrain cell (sub_43DD50), read the dialog's channel checkboxes + mode,
//  seed the per-channel paint state, and dispatch to sub_43E4F0 -> PMESH_16.
// ═══════════════════════════════════════════════════════════════════════════════
typedef float ( *PaintCallback )( float *cp, int channel, float cur, float strength, float weight );

extern char  sub_43DD50( const float *dir, unsigned char *colorOut, const float *cam_origin, float *origin_out ); // pmesh.cpp
extern void  sub_43E4F0( PaintCallback cb, float *center, char channelMask, float cellInfo ); // pmesh.cpp
extern float sub_43E550( float *, int, float, float, float );   // raise/lower
extern float sub_43E570( float *, int, float, float, float );   // set toward target
extern float sub_43E5D0( float *, int, float, float, float );   // average gather
extern float sub_43E610( float *, int, float, float, float );   // average apply
extern float sub_43E670( float *, int, float, float, float );   // noise
extern float sub_401BB0();                                      // inner radius getter (pmesh.cpp)
extern float sub_401C00();                                      // outer radius getter (pmesh.cpp)
extern float sub_401C50();                                      // strength getter (pmesh.cpp)
extern float g_paintChannelVal[5];                              // flt_231F534[0..4] (pmesh.cpp)
extern float g_paintChannelWt[5];                               // flt_231F520[0..4] (pmesh.cpp)
extern float grid_sizes[];                                      // engine_stubs

// The paint TARGET colour (the dialog's colour control / the case-5 eyedropper); packed
// {B, G, R} like the IDB dword_25D65A4, with a separate alpha byte_25D65A8.  Default white.
unsigned int  g_paintColorBGR = 0x00FFFFFFu;   // 0x25D65A4
unsigned char g_paintColorA   = 0xFFu;         // 0x25D65A8

// Is an AdvPatchEditDlg control checked? (live read, BM_GETCHECK = WM_USER... = 0xF0).
static bool AdvDlg_IsChecked( int id )
{
    CWnd *dlg = AdvPatchEdit_GetDlg();
    if ( !dlg || !dlg->GetSafeHwnd() )
        return false;
    CWnd *c = dlg->GetDlgItem( id );
    return c && c->SendMessage( BM_GETCHECK, 0, 0 ) != 0;
}

// sub_401DB0 (0x401DB0) — return the checked mode radio's index (6 = none/off).
int sub_401DB0()
{
    static const struct { int id, mode; } s_modes[7] =
        { { 1435, 0 }, { 1439, 1 }, { 1437, 2 }, { 1436, 3 }, { 1438, 4 }, { 1468, 5 }, { 1440, 6 } };
    for ( int i = 0; i < 7; ++i )
        if ( AdvDlg_IsChecked( s_modes[i].id ) )
            return s_modes[i].mode;
    return 6;
}

// sub_401D50 (0x401D50) - "is terrain-paint mode active?" gate (Drag_Begin).
// IDA gates on the live AdvPatchEditDlg controls: mode != Off and outer radius > inner
// radius.  It does not require the modeless dialog object's top window to report visible.
int sub_401D50()
{
    if ( sub_401DB0() == 6 )
        return 0;                                   // mode "Off"
    return ( sub_401C00() > sub_401BB0() ) ? 1 : 0; // outer radius > inner radius
}

// sub_43E6F0 (0x43E6F0) — the addpoint paint drag.  buttons 1=raise/LMB, 2=lower/RMB;
// origin/dir are float* ray endpoints (int-cast by Drag_MouseMoved).
void sub_43E6F0( int buttons, int origin, int dir )
{
    if ( buttons != 1 && buttons != 2 )
        return;
    float *org = (float *)(intptr_t)origin;
    float *dr  = (float *)(intptr_t)dir;

    float center[3];
    if ( !sub_43DD50( dr, nullptr, org, center ) )      // pick the terrain cell under the cursor
        return;
    const float sign = ( buttons == 1 ) ? 1.0f : -1.0f;

    // Channel mask from the dialog checkboxes (1469 Height, 1470 Color-all, 1471/72/73 B/G/R, 1474 A).
    int mask = AdvDlg_IsChecked( 1469 ) ? 1 : 0;
    if ( AdvDlg_IsChecked( 1470 ) )
        mask |= 0xE;
    else
    {
        if ( AdvDlg_IsChecked( 1471 ) ) mask |= 2;
        if ( AdvDlg_IsChecked( 1472 ) ) mask |= 4;
        if ( AdvDlg_IsChecked( 1473 ) ) mask |= 8;
    }
    if ( AdvDlg_IsChecked( 1474 ) )
        mask |= 0x10;

    switch ( sub_401DB0() )
    {
    case 0:   // raise / lower height
    {
        float amount = grid_sizes[g_qeglobals.d_gridsize] * sign * 0.5f;
        sub_43E4F0( sub_43E550, center, (char)mask, amount );
        break;
    }
    case 2:   // set colour toward the paint target
        // [DEFERRED] CurvEditDlg::OnHeightText2() — dialog text refresh (cosmetic).
        g_paintChannelVal[0] = sign;
        if ( sign >= 0.0f )
        {
            g_paintChannelVal[1] = (float)( unsigned char )( g_paintColorBGR );         // B
            g_paintChannelVal[2] = (float)( unsigned char )( g_paintColorBGR >> 8 );    // G
            g_paintChannelVal[3] = (float)( unsigned char )( g_paintColorBGR >> 16 );   // R
            g_paintChannelVal[4] = (float)g_paintColorA;                                // A
        }
        else
            g_paintChannelVal[1] = g_paintChannelVal[2] = g_paintChannelVal[3] = g_paintChannelVal[4] = 255.0f;
        sub_43E4F0( sub_43E570, center, (char)mask, 1.0f );
        break;

    case 3:   // average (gather pass -> normalise -> apply pass)
        for ( int k = 0; k < 5; ++k ) { g_paintChannelVal[k] = 0.0f; g_paintChannelWt[k] = 0.0f; }
        sub_43E4F0( sub_43E5D0, center, (char)mask, 0.0f );
        if ( g_paintChannelWt[0] == 0.0f ) mask &= ~1;    else g_paintChannelVal[0] /= g_paintChannelWt[0]; // height
        if ( g_paintChannelWt[1] == 0.0f ) mask &= ~2;    else g_paintChannelVal[1] /= g_paintChannelWt[1]; // B
        if ( g_paintChannelWt[2] == 0.0f ) mask &= ~4;    else g_paintChannelVal[2] /= g_paintChannelWt[2]; // G
        if ( g_paintChannelWt[3] == 0.0f ) mask &= ~8;    else g_paintChannelVal[3] /= g_paintChannelWt[3]; // R
        if ( g_paintChannelWt[4] == 0.0f ) mask &= ~0x10; else g_paintChannelVal[4] /= g_paintChannelWt[4]; // A
        if ( mask )
        {
            float amount = sub_401C50() * 0.009999999776482582f * grid_sizes[g_qeglobals.d_gridsize];
            sub_43E4F0( sub_43E610, center, (char)mask, amount );
        }
        break;

    case 4:   // noise: seed every channel's w-offset to the per-stroke counter, then apply
    {
        // flt_2665724 — a global float counter, +1 per stroke, so each stroke samples a
        // different 4D-noise slice (g_paintChannelWt[c] is the noise w-coord in sub_43E670).
        static float g_noiseStrokeW = 0.0f;                 // 0x2665724 (init 0.0)
        for ( int k = 0; k < 5; ++k ) g_paintChannelWt[k] = g_noiseStrokeW;
        g_noiseStrokeW += 1.0f;
        float amount = sub_401C50();                        // strength (raw, unscaled)
        sub_43E4F0( sub_43E670, center, (char)mask, amount );
        break;
    }

    case 5:   // eyedropper: pick the cell's height + colour into the paint target
    {
        unsigned char picked[4] = { 0, 0, 0, 0 };
        if ( sub_43DD50( dr, picked, org, center ) )
        {
            g_paintChannelVal[0] = center[2];          // picked height
            g_paintChannelVal[1] = (float)picked[2];   // B
            g_paintChannelVal[2] = (float)picked[1];   // G
            g_paintChannelVal[3] = (float)picked[0];   // R
            g_paintChannelVal[4] = (float)picked[3];   // A
            AdvPatchEdit_ShowHeight( center[2] );      // CurvEditDlg::OnHeightText(picked height)
            g_paintColorBGR = picked[2] | ( picked[1] << 8 ) | ( picked[0] << 16 );   // {B,G,R}
            g_paintColorA   = picked[3];
        }
        break;
    }

    default:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CPatchDialog — the Patch Inspector "Patch Properties" dialog (binary CWnd_PatchDialog
//  0x25D7278, PE resource 0xAC = IDD_PATCH_PROPERTIES).  A MODELESS CDialog.  This piece
//  ports the DETAILS half READ-ONLY: pick a control point with the Row/Column combos and
//  show its X/Y/Z + current-layer S/T.  The Type combo, the Apply write-back, and the whole
//  Texturing half (texdef edits/spins + CAP/Set/Natural/Fit) are later pieces.
//    DoPatchInspector (0x436d30) / g_PatchDialog.GetPatchInfo (0x436ba0) / sub_436E10
//    (0x436e10) / UpdatePatchInspector (0x436db0).
// ═══════════════════════════════════════════════════════════════════════════════
extern selbrush_t selected_brushes;     // qe3.h
extern int        QE_SingleBrush();      // select.cpp
extern int        g_nUpdateBits;         // pmesh.cpp (0x25D5A74) pending window-update mask
extern void       Patch_NaturalizeSelected( bool unk, bool cap, float x, float y );  // pmesh.cpp 0x447fd0
extern void       Brush_FitTexture( float x, float y, int a4 );                       // select.cpp 0x4939e0
extern void       Patch_SetTextureInfo( texdef_sub_t *texDef );                       // pmesh.cpp 0x447760
extern void       Patch_SetTexturing( float sx, float sy, int mode );                 // pmesh.cpp 0x446b60

// ── CTextureLayout (IDD_PATCH_TEXTURE_LAYOUT / resource 0x9F) ──────────────────────
//  The modal "Patch texture layout" dialog opened by the Patch Inspector's "Set..." button
//  (sub_459860 ctor / sub_436980).  Enter a texture x/y scale (default 4) + pick a layout mode,
//  then OK -> Patch_SetTexturing(x, y, mode).  Modes: 2D distance->0 (natural), 3D distance->1
//  (arc-length), emulates curves->2 (grid).
class CTextureLayout : public CDialog
{
public:
    CTextureLayout( CWnd *parent ) : CDialog( IDD_PATCH_TEXTURE_LAYOUT, parent ),
        m_x( 4.0f ), m_y( 4.0f ), m_mode( 0 ) {}
    float m_x, m_y;     // texture x/y scale (ctor defaults 4.0, sub_459860 +116/+120)
    int   m_mode;       // 0=2D distance, 1=3D distance, 2=emulates curves (+124)

protected:
    virtual BOOL OnInitDialog()
    {
        CDialog::OnInitDialog();
        SetDlgItemText( 1089, "4" );  SetDlgItemText( 1142, "4" );
        CheckDlgButton( 1449, BST_CHECKED );    // default: 2D distance (good for ground)
        return TRUE;
    }
    virtual void OnOK()
    {
        CString sx, sy;
        GetDlgItemText( 1089, sx );  GetDlgItemText( 1142, sy );
        m_x = (float)atof( (LPCSTR)sx );
        m_y = (float)atof( (LPCSTR)sy );
        m_mode = IsDlgButtonChecked( 1475 ) ? 1 : ( IsDlgButtonChecked( 1476 ) ? 2 : 0 );
        CDialog::OnOK();
    }
};

class CPatchInspectorDlg : public CDialog
{
public:
    CPatchInspectorDlg() : CDialog( IDD_PATCH_PROPERTIES, nullptr ), m_def( nullptr ) {}
    patchMesh_t *m_def;                  // the inspected patch (binary this+116)

protected:
    void SetField( int id, float v ) { CString s; s.Format( "%g", v ); SetDlgItemText( id, s ); }
    void FillCombo( int id, int count )
    {
        SendDlgItemMessage( id, CB_RESETCONTENT, 0, 0 );
        for ( int i = 0; i < count; ++i )
        {
            char s[16]; sprintf( s, "%i", i );
            SendDlgItemMessage( id, CB_ADDSTRING, 0, (LPARAM)s );
        }
        if ( count > 0 ) SendDlgItemMessage( id, CB_SETCURSEL, 0, 0 );
    }
    // sub_436E10 — show the selected control point's X/Y/Z + current-layer S/T.
    void ShowSelectedPoint()
    {
        static const int fields[5] = { 1089, 1142, 1147, 1102, 1106 };   // X Y Z S T
        for ( int i = 0; i < 5; ++i )
            SetDlgItemText( fields[i], "" );
        if ( !m_def )
            return;
        int row = (int)SendDlgItemMessage( 1301, CB_GETCURSEL, 0, 0 );   // Row combo (0..height-1)
        int col = (int)SendDlgItemMessage( 1302, CB_GETCURSEL, 0, 0 );   // Column combo (0..width-1)
        if ( row < 0 || row >= m_def->height || col < 0 || col >= m_def->width )
            return;
        drawVert_t *cp = &m_def->ctrl[col][row];                         // idx row+16*col
        SetField( 1089, cp->xyz[0] );  SetField( 1142, cp->xyz[1] );  SetField( 1147, cp->xyz[2] );
        const float *st = (const float *)( (const char *)&cp->texCoord + 8 * g_qeglobals.current_edit_layer );
        SetField( 1102, st[0] );  SetField( 1106, st[1] );
    }

public:
    // g_PatchDialog.GetPatchInfo — bind to the selected single patch + repopulate the combos.
    void GetPatchInfo()
    {
        patch_t *patch = nullptr;
        if ( QE_SingleBrush() && ( patch = selected_brushes.next->patch ) != nullptr )
            m_def = patch->def;
        else
            m_def = nullptr;
        FillCombo( 1301, m_def ? m_def->height : 0 );   // Row    0..height-1
        FillCombo( 1302, m_def ? m_def->width  : 0 );   // Column 0..width-1
        ShowSelectedPoint();
    }

    float GetFieldF( int id ) { CString s; GetDlgItemText( id, s ); return (float)atof( (LPCSTR)s ); }

protected:
    virtual BOOL OnInitDialog()
    {
        CDialog::OnInitDialog();
        // Texdef spin STEP defaults (binary ctor flt_25D75A8..75B8): stretch/shift 0.05, rotate 45.
        SetDlgItemText( 1211, "0.05" );  SetDlgItemText( 1195, "0.05" );  SetDlgItemText( 1217, "45" );
        SetDlgItemText( 1213, "0.05" );  SetDlgItemText( 1196, "0.05" );
        GetPatchInfo();
        return TRUE;
    }
    afx_msg void OnRowColChange() { ShowSelectedPoint(); }
    // OnApply (0x436ef0) — write the edited X/Y/Z + current-layer S/T back into the selected
    // control point, bump the patch version, and force a redraw (no explicit Patch_Rebuild —
    // the version bump + g_nUpdateBits=-1 re-tessellate on the next render, as in the binary).
    afx_msg void OnApply()
    {
        if ( !m_def )
            return;
        int row = (int)SendDlgItemMessage( 1301, CB_GETCURSEL, 0, 0 );
        int col = (int)SendDlgItemMessage( 1302, CB_GETCURSEL, 0, 0 );
        if ( row < 0 || row >= m_def->height || col < 0 || col >= m_def->width )
            return;
        drawVert_t *cp = &m_def->ctrl[col][row];
        cp->xyz[0] = GetFieldF( 1089 );  cp->xyz[1] = GetFieldF( 1142 );  cp->xyz[2] = GetFieldF( 1147 );
        float *st = (float *)( (char *)&cp->texCoord + 8 * g_qeglobals.current_edit_layer );
        st[0] = GetFieldF( 1102 );  st[1] = GetFieldF( 1106 );
        ++m_def->version;             // +0x5040
        g_nUpdateBits = -1;           // force a full redraw (re-tessellates the dirty patch)
    }
    // Texturing buttons — thin wrappers over the ported patch-texturing ops, scaled by the
    // current edit layer's sample size (random_texture_stuff[layer].sampleSize, IDB +36).
    afx_msg void OnCap()                          // CAP (1282) -> sub_4368D0
    {
        float s = g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].sampleSize;
        Patch_NaturalizeSelected( true, false, s, s );
        g_nUpdateBits = -1;
    }
    afx_msg void OnNatural()                       // Natural (1284) -> sub_436940
    {
        float s = g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].sampleSize;
        Patch_NaturalizeSelected( false, false, s, s );
        g_nUpdateBits = -1;
    }
    afx_msg void OnFit()                           // Fit (1286) -> sub_436910
    {
        Brush_FitTexture( 1.0f, 1.0f, 0 );
        g_nUpdateBits = -1;
    }
    afx_msg void OnSetTexturing()                  // "Set..." (1283) -> sub_436980
    {
        CTextureLayout dlg( this );                // modal "Patch texture layout"
        if ( dlg.DoModal() == IDOK )
            Patch_SetTexturing( dlg.m_x, dlg.m_y, dlg.m_mode );
        g_nUpdateBits = -1;
    }
    // Texdef spin arrows (1248/1251/1254/1257/1259) — apply a RELATIVE texture transform
    // by the step typed in the matching edit (sub_4370E0 -> Patch_SetTextureInfo): shift =
    // +/-step, stretch = *(1 -/+ step), rotate = +/-step.  Up = iDelta > 0.
    afx_msg void OnSpin( UINT id, NMHDR *pNMHDR, LRESULT *pResult )
    {
        bool up = ( (NMUPDOWN *)pNMHDR )->iDelta > 0;
        texdef_sub_t td;
        memset( &td, 0, sizeof( td ) );
        switch ( id )
        {
        case 1259: { float s = GetFieldF( 1217 ); td.rotate  = up ?  s        : -s;        break; } // Rotate
        case 1257: { float s = GetFieldF( 1211 ); td.size[0] = up ? 1.0f - s  : 1.0f + s;  break; } // Horiz stretch
        case 1254: { float s = GetFieldF( 1213 ); td.size[1] = up ? 1.0f - s  : 1.0f + s;  break; } // Vert  stretch
        case 1248: { float s = GetFieldF( 1195 ); td.shift[0] = up ?  s        : -s;        break; } // Horiz shift
        case 1251: { float s = GetFieldF( 1196 ); td.shift[1] = up ?  s        : -s;        break; } // Vert  shift
        default:   *pResult = 0; return;
        }
        Patch_SetTextureInfo( &td );
        g_nUpdateBits |= 1;
        *pResult = 0;
    }
    virtual void OnOK()           { ShowWindow( SW_HIDE ); }   // "Done" (IDOK): modeless -> hide
    virtual void OnCancel()       { ShowWindow( SW_HIDE ); }
    afx_msg void OnDestroy();     // 0x436DD0 — save "Radiant::PatchWindow" (audit U8/D4)
    DECLARE_MESSAGE_MAP()
};

// CPatchInspectorDlg OnDestroy (0x436DD0) — persist the dialog rect.  [audit U8/D4 —
// handler was absent; the load side is in DoPatchInspector below.]
extern BOOL SaveRegistryInfo( const char *pszName, void *pvBuf, int lSize );   // win_qe3.cpp
extern BOOL LoadRegistryInfo( const char *pszName, void *pvBuf, long *plSize );
void CPatchInspectorDlg::OnDestroy()
{
    if ( GetSafeHwnd() )
    {
        RECT rect;
        ::GetWindowRect( GetSafeHwnd(), &rect );             // 0x436DD0–0x436E0D
        SaveRegistryInfo( "Radiant::PatchWindow", &rect, 0x10 );
    }
    CDialog::OnDestroy();
}

BEGIN_MESSAGE_MAP( CPatchInspectorDlg, CDialog )
    ON_WM_DESTROY()
    ON_CBN_SELCHANGE( 1301, OnRowColChange )
    ON_CBN_SELCHANGE( 1302, OnRowColChange )
    ON_BN_CLICKED( 3, OnApply )            // "Apply" button (0x436ef0)
    ON_BN_CLICKED( 1282, OnCap )           // CAP     (0x4368d0)
    ON_BN_CLICKED( 1284, OnNatural )       // Natural (0x436940)
    ON_BN_CLICKED( 1286, OnFit )           // Fit     (0x436910)
    ON_BN_CLICKED( 1283, OnSetTexturing )  // Set...  (0x436980 -> CTextureLayout)
    ON_NOTIFY_RANGE( UDN_DELTAPOS, 1248, 1259, OnSpin )   // texdef spins (0x436ae0 -> sub_4370E0)
END_MESSAGE_MAP()

static CPatchInspectorDlg *g_pPatchInspector = nullptr;

// DoPatchInspector (0x436d30) — create (first use) + show the modeless inspector, then populate.
void DoPatchInspector()
{
    if ( !g_pPatchInspector )
    {
        g_pPatchInspector = new CPatchInspectorDlg();
        g_pPatchInspector->Create( IDD_PATCH_PROPERTIES, AfxGetMainWnd() );
        // Restore the saved position (binary DoPatchInspector 0x436d30:
        // LoadRegistryInfo("Radiant::PatchWindow") → SetWindowPos(x,y,SWP_NOSIZE)).
        RECT rect;  long size = sizeof( rect );              // [audit U8/D4]
        if ( LoadRegistryInfo( "Radiant::PatchWindow", &rect, &size ) && size >= (long)sizeof( rect ) )
            g_pPatchInspector->SetWindowPos( nullptr, rect.left, rect.top, 0, 0,
                                             SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
    }
    g_pPatchInspector->ShowWindow( SW_SHOW );
    g_pPatchInspector->GetPatchInfo();
}

// UpdatePatchInspector (0x436db0) — refresh the inspector if it is open.
void UpdatePatchInspector()
{
    if ( g_pPatchInspector && g_pPatchInspector->GetSafeHwnd() && g_pPatchInspector->IsWindowVisible() )
        g_pPatchInspector->GetPatchInfo();
}

// Global Patch Inspector accessors used by brush, patch, and selection refresh paths.
CWnd *g_PatchDialog_GetHwnd()
{
    return ( g_pPatchInspector && g_pPatchInspector->GetSafeHwnd() ) ? g_pPatchInspector : nullptr;
}
void g_PatchDialog_GetPatchInfo()
{
    if ( g_pPatchInspector && g_pPatchInspector->GetSafeHwnd() )
        g_pPatchInspector->GetPatchInfo();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CPatchDensityDlg — "Patch density" (Curve → Simple Patch Mesh, menu cmd 32856).
//  A MODAL CDialog over IDD_PATCH_DENSITY (157).  Two CBS_DROPDOWNLIST combos
//  (1280 width / 1281 height) choose the patch density from the binary's dimension
//  table {3,5,7,9,11,13,15}; OK calls Patch_GenericMesh over the selected brush.
//
//  IDB cluster (all renamed in the IDB this session):
//    CPatchDensityDlg_Init          0x436280 — ctor (IDD 0x9D, 2 CComboBox @ +0x74/+0xC8)
//    CPatchDensityDlg_DoDataExchange 0x436330 — DDX_Control(1280→combo0, 1281→combo1)
//    CPatchDensityDlg_OnInitDialog  0x436440 — base OnInitDialog + CB_SETCURSEL(last sel)
//    CPatchDensityDlg_OnOK          0x436390 — read CB_GETCURSEL → dims[] → Patch_GenericMesh
//    CMainFrame::OnCurveSimplepatchmesh 0x429a20 — Undo bracket + DoModal + destruct
//
//  FIDELITY NOTE: the binary's OnInitDialog only CB_SETCURSELs the last selection —
//  it never adds the combo items, and the dialog template (PE rsrc 157, ground-truthed)
//  carries empty combos.  i.e. the stock dialog shows EMPTY dropdowns (the menu command
//  is also un-wired/auto-greyed in stock, per PROGRESS.md).  Since the combos must hold
//  the dims for CB_GETCURSEL→dims[sel] to mean anything, we POPULATE them from the
//  binary's own g_patchDensityDims table {3,5,7,9,11,13,15} in OnInitDialog (the
//  GtkRadiant DoNewPatchDlg confirms these exact 7 items + active index 0) — the only
//  added line vs the binary, so the feature actually works.
// ═══════════════════════════════════════════════════════════════════════════════
#include "mainfrm.h"                  // CMainFrame / CXYWnd (m_pActiveXY->m_nViewType)
extern CMainFrame *g_pParentWnd;      // mainfrm.cpp (0x25D5A70)
extern int         Sys_Printf( const char *fmt, ... );          // win_qe3.cpp (0x499E90)
extern int         QE_SingleBrush();                            // qe3.cpp (0x48C8B0)
extern selbrush_t  selected_brushes;                            // engine_stubs (0x23F1864)
extern void        Undo_ClearRedo();                            // undo.cpp
extern void        Undo_GeneralStart( const char *operation );  // undo.cpp
extern void        Undo_AddBrushList( selbrush_t *sb );         // undo.cpp
extern void        Undo_EndBrushList( selbrush_t *sb );         // undo.cpp
extern void        Undo_End();                                  // undo.cpp
extern selbrush_t *Patch_GenericMesh( int nWidth, int nHeight, int nOrientation,
                                      char bDeleteSource, char bOverwrite );   // pmesh.cpp (0x43b310)
extern selbrush_t *Patch_GenericTerrainMesh( int nWidth, int nHeight, int nOrientation,
                                             char bDeleteSource, char bOverwrite );

// The binary's dimension table (g_patchDensityDims @0x73B170): combo index → patch size.
static const int s_patchDensityDims[7] = { 3, 5, 7, 9, 11, 13, 15 };

// Last combo selections, remembered across opens (IDB g_patchDensityLastWidthSel 0x25D5AFC /
// g_patchDensityLastHeightSel 0x25D5B00; both 0 = "3" on first open).
static int g_patchDensityLastWidthSel  = 0;
static int g_patchDensityLastHeightSel = 0;

class CPatchDensityDlg : public CDialog
{
public:
    CPatchDensityDlg( bool terrain, CWnd *parent = nullptr )
        : CDialog( IDD_PATCH_DENSITY, parent ), m_terrain( terrain ) {}

protected:
    CComboBox m_wndWidth;   // this+0x74 (DDX 1280)
    CComboBox m_wndHeight;  // this+0xC8 (DDX 1281)
    bool m_terrain;

    virtual void DoDataExchange( CDataExchange *pDX )       // 0x436330
    {
        CDialog::DoDataExchange( pDX );
        DDX_Control( pDX, 1280, m_wndWidth );
        DDX_Control( pDX, 1281, m_wndHeight );
    }

    virtual BOOL OnInitDialog()                            // 0x436440
    {
        CDialog::OnInitDialog();
        // (see FIDELITY NOTE) populate both combos from the dims table, then restore
        // the last selection (the binary's two CB_SETCURSELs).
        for ( int i = 0; i < 7; ++i )
        {
            char s[16];
            sprintf( s, "%d", s_patchDensityDims[i] );
            m_wndWidth.AddString( s );
            m_wndHeight.AddString( s );
        }
        m_wndWidth.SetCurSel( g_patchDensityLastWidthSel );
        m_wndHeight.SetCurSel( g_patchDensityLastHeightSel );
        return TRUE;
    }

    virtual void OnOK()                                    // 0x436390
    {
        int wSel = m_wndWidth.GetCurSel();
        int hSel = m_wndHeight.GetCurSel();
        if ( (unsigned int)wSel <= 6 && (unsigned int)hSel <= 6 )
        {
            int nHeight = s_patchDensityDims[hSel];
            if ( nHeight < 3 )
                Sys_Printf( "Invalid patch width or height.\n" );
            else
            {
                selbrush_t *( *createPatch )( int, int, int, char, char ) =
                    m_terrain ? Patch_GenericTerrainMesh : Patch_GenericMesh;
                createPatch( s_patchDensityDims[wSel], nHeight,
                             g_pParentWnd->m_pActiveXY->m_nViewType, 1, 0 );
            }
            g_nUpdateBits = -1;
        }
        // remember the selection (IDB stores edi/ebx into the two globals).
        g_patchDensityLastWidthSel  = wSel;
        g_patchDensityLastHeightSel = hSel;
        CDialog::OnOK();
    }
};

// CMainFrame::OnCurveSimplepatchmesh (0x429a20, menu cmd 32856 / 0x8058 — verified from
// the CMainFrame command table {cmdId,cmdId,0x38,pfn}).  Brackets the patch create in a
// single Undo op and runs the density dialog modally.
void DoSimpleTerrainPatchMesh( bool terrain )
{
    if ( !QE_SingleBrush() )
        return;
    Undo_ClearRedo();
    Undo_GeneralStart( terrain ? "make simple terrain patch" : "make simple patch mesh" );
    Undo_AddBrushList( &selected_brushes );
    {
        CPatchDensityDlg dlg( terrain );
        dlg.DoModal();
    }
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

void DoSimplePatchMesh()
{
    DoSimpleTerrainPatchMesh( false );
}
