#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Modeless patch vertex color/alpha editor. Apply writes enabled channels to
// selected control points and rebuilds each changed patch under Undo.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <afxdlgs.h>   // CColorDialog — TU-LOCAL ONLY (adding it to the shared stdafx.h PCH
                       // reorders windows.h ahead of the DXSDK headers and breaks every
                       // gfx_d3d TU — see the win_ent.cpp model-picker build gotcha).
#include <cstring>

extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ── externs (per-TU, matching pmesh.cpp / undo.cpp) ───────────────────────────
extern undo_s   *g_lastundo;                                   // 0x23F162C (undo.cpp)
extern void      Undo_AddBrush( entity_brush_s *pBrushInst );  // 0x45E680 (undo.cpp)
extern void      Undo_AddEntity( int a1 );                     // 0x45E8B0 (undo.cpp)
extern void      Patch_Rebuild( patchMesh_t *p, char doBounds );// 0x438D80 (pmesh.cpp)
extern int       Sys_Printf( const char *fmt, ... );           // 0x499E90
extern int       g_nUpdateBits;                                // 0x25D5A74 (engine_stubs.cpp)
// selected_brushes sentinel + g_qeglobals are declared in qe3.h.

// The Undo bracket the binary opens on the FIRST recoloured control point of a patch
// (0x461210 inner: g_lastundo==0 ? "no last undo" : the Undo_AddBrushList idiom).  Sets the
// patch's xx22b "already-bracketed" flag so subsequent points on the same patch don't
// re-bracket.  Returns true once the patch is bracketed (so the apply marks it changed).
static void VED_BracketPatch( patchMesh_t *patch )
{
    if ( patch->xx22b )
        return;
    patch->xx22b = true;

    if ( !g_lastundo )
    {
        Sys_Printf( "Undo_AddBrush: no last undo.\n" );
        return;
    }
    if ( g_lastundo->entitylist.next != &g_lastundo->entitylist )
        Sys_Printf( "Undo_AddBrush: WARNING adding brushes after entity.\n" );

    entity_brush_s *pSymbiot = patch->pSymbiot;          // the symbiont brush DEF node
    entity_s_def   *owner    = (entity_s_def *)pSymbiot->owner;
    if ( *(int *)&owner->eclass->fixedsize )             // owner->eclass->fixedsize (eclass@0x60, fixedsize@0x08)
        Undo_AddEntity( (int)(intptr_t)owner );
    Undo_AddBrush( pSymbiot );
}

// ═════════════════════════════════════════════════════════════════════════════
//  THE CORE — 0x461210  CVertEditDlg::Apply (VertEditDlg_01)
//  Paint (R,G,B) and/or A onto the selected patch control points.
//    doColour → write vert_color.{r,g,b};  doAlpha → write vert_color.a.
// ═════════════════════════════════════════════════════════════════════════════
void VertEditDlg_Apply( unsigned char r, unsigned char g, unsigned char b, unsigned char a,
                        bool doColour, bool doAlpha )
{
    for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
    {
        patch_t *pInst = sb->patch;
        if ( !pInst )
            continue;

        // 0x46123C: the `b->patch->def == b->def->patch` invariant (type 0 → log+continue).
        if ( pInst->def != sb->def->patch )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\VertEditDlg.cpp",
                    199, 0, "%s", "b->patch->def == b->def->patch" );

        patchMesh_t *patch = sb->def->patch;             // the DEF grid (== pInst->def)
        bool changed = false;

        for ( int col = 0; col < patch->width; ++col )
            for ( int row = 0; row < patch->height; ++row )
            {
                drawVert_t *cp = &patch->ctrl[col][row];
                if ( cp->turned_edge & 2 )               // 0x4C: turned edge → skip
                    continue;

                // Only picked (move-point) control points (0x461272: scan d_move_points).
                bool picked = false;
                for ( int m = 0; m < g_qeglobals.d_num_move_points; ++m )
                    if ( cp == g_qeglobals.d_move_points[m] ) { picked = true; break; }
                if ( !picked )
                    continue;

                if ( doColour )                          // SendMessageA(this+600, BM_GETCHECK)
                {
                    VED_BracketPatch( patch );
                    changed = true;
                    cp->vert_color.b = b;                // this+120
                    cp->vert_color.g = g;                // this+124
                    cp->vert_color.r = r;                // this+128
                }
                if ( doAlpha )                           // SendMessageA(this+516, BM_GETCHECK)
                {
                    VED_BracketPatch( patch );
                    changed = true;
                    cp->vert_color.a = a;                // this+116
                }
            }

        if ( changed )                                   // 0x4611C8: rebuild touched patch
            Patch_Rebuild( patch, 1 );
    }

    g_nUpdateBits = -1;
}

// ══════════════════════════════════════════════════════════════════════════════
//  CVertEditDlg — the hand-built modeless popup (CVehicleDlg / CModelDlg pattern)
// ══════════════════════════════════════════════════════════════════════════════
//  4 R/G/B/A sliders (0..255), a [Color...] picker, two enable checkboxes (Color / Alpha),
//  and [Apply].  Apply pushes the slider RGBA onto the picked patch control points.
//  radiant.rc has no IDD template, so it is hand-built (the established sibling pattern).

enum
{
    IDC_VED_R_SLIDER = 4500,    // R/G/B/A track bars (binary CSliderCtrl members)
    IDC_VED_G_SLIDER,
    IDC_VED_B_SLIDER,
    IDC_VED_A_SLIDER,
    IDC_VED_CHK_COLOR,          // enable writing RGB    (binary CButton @this+600)
    IDC_VED_CHK_ALPHA,          // enable writing A      (binary CButton @this+516)
    IDC_VED_COLOR_BTN,          // [Color...] → CColorDialog (binary sub_461480)
    IDC_VED_APPLY,              // [Apply] → VertEditDlg_Apply
};

CVertEditDlg *g_dlgVertEdit = nullptr;      // the singleton (NULL until first opened)

static HFONT s_vedFont    = nullptr;
static HWND  s_vedR       = nullptr;        // slider HWNDs
static HWND  s_vedG       = nullptr;
static HWND  s_vedB       = nullptr;
static HWND  s_vedA       = nullptr;
static HWND  s_vedChkClr  = nullptr;        // enable checkboxes
static HWND  s_vedChkAlp  = nullptr;
static HWND  s_vedSwatch  = nullptr;        // colour preview static

// The dialog's stored colour (the binary's dword_25D6624.. members; ctor seeds 255/255/0/0).
static unsigned char s_vedColR = 255;
static unsigned char s_vedColG = 255;
static unsigned char s_vedColB = 0;
static unsigned char s_vedColA = 0;

static HWND VED_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                           int x, int y, int w, int hgt, const char *text = nullptr )
{
    HWND h = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                              x, y, w, hgt, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_vedFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_vedFont, 0 );
    return h;
}

// Pull the current slider positions into the stored colour, then repaint the swatch.
static void VED_SyncFromSliders()
{
    if ( s_vedR ) s_vedColR = (unsigned char)SendMessageA( s_vedR, TBM_GETPOS, 0, 0 );
    if ( s_vedG ) s_vedColG = (unsigned char)SendMessageA( s_vedG, TBM_GETPOS, 0, 0 );
    if ( s_vedB ) s_vedColB = (unsigned char)SendMessageA( s_vedB, TBM_GETPOS, 0, 0 );
    if ( s_vedA ) s_vedColA = (unsigned char)SendMessageA( s_vedA, TBM_GETPOS, 0, 0 );
    if ( s_vedSwatch )
        ::InvalidateRect( s_vedSwatch, nullptr, TRUE );
}

static void VED_PushToSliders()
{
    if ( s_vedR ) SendMessageA( s_vedR, TBM_SETPOS, TRUE, s_vedColR );
    if ( s_vedG ) SendMessageA( s_vedG, TBM_SETPOS, TRUE, s_vedColG );
    if ( s_vedB ) SendMessageA( s_vedB, TBM_SETPOS, TRUE, s_vedColB );
    if ( s_vedA ) SendMessageA( s_vedA, TBM_SETPOS, TRUE, s_vedColA );
    if ( s_vedSwatch )
        ::InvalidateRect( s_vedSwatch, nullptr, TRUE );
}

BEGIN_MESSAGE_MAP( CVertEditDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_HSCROLL()
    ON_BN_CLICKED( IDC_VED_COLOR_BTN, &CVertEditDlg::OnColorButton )
    ON_BN_CLICKED( IDC_VED_APPLY,     &CVertEditDlg::OnApply )
END_MESSAGE_MAP()

CVertEditDlg::CVertEditDlg()
{
}

int CVertEditDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_vedFont )
        s_vedFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 10, lblW = 24, slW = 220, rowH = 30;
    int lx = M, sx = M + lblW + 4, y = M;

    struct { const char *lbl; HWND *out; int id; unsigned char init; } rows[] = {
        { "R", &s_vedR, IDC_VED_R_SLIDER, s_vedColR },
        { "G", &s_vedG, IDC_VED_G_SLIDER, s_vedColG },
        { "B", &s_vedB, IDC_VED_B_SLIDER, s_vedColB },
        { "A", &s_vedA, IDC_VED_A_SLIDER, s_vedColA },
    };
    for ( int n = 0; n < 4; ++n )
    {
        VED_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, rows[n].lbl );
        HWND s = VED_MakeChild( self, TRACKBAR_CLASS, TBS_HORZ | TBS_AUTOTICKS | WS_TABSTOP,
                                rows[n].id, sx, y, slW, 24 );
        if ( s )
        {
            // TBM_SETRANGE lParam: LOWORD = min (0), HIWORD = max (255).
            ::SendMessageA( s, TBM_SETRANGE, TRUE, (LPARAM)( (WORD)0 | ( (DWORD)255 << 16 ) ) );
            ::SendMessageA( s, TBM_SETPOS,   TRUE, rows[n].init );
        }
        *rows[n].out = s;
        y += rowH;
    }

    // colour swatch preview (owner-drawn static fill in OnCtlColor below).
    s_vedSwatch = VED_MakeChild( self, "static", SS_LEFT | WS_BORDER, 0, sx + slW + 10, M, 40, 4 * rowH - 6 );

    y += 6;
    s_vedChkClr = VED_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_VED_CHK_COLOR, lx, y, 120, 20, "Apply Color" );
    s_vedChkAlp = VED_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_VED_CHK_ALPHA, lx + 130, y, 120, 20, "Apply Alpha" );
    if ( s_vedChkClr ) ::SendMessageA( s_vedChkClr, BM_SETCHECK, BST_CHECKED, 0 );   // colour on by default
    y += rowH;

    VED_MakeChild( self, "button", BS_PUSHBUTTON  | WS_TABSTOP, IDC_VED_COLOR_BTN, lx, y, 90, 24, "Color..." );
    VED_MakeChild( self, "button", BS_DEFPUSHBUTTON | WS_TABSTOP, IDC_VED_APPLY,    lx + 100, y, 90, 24, "Apply" );
    return 0;
}

// Slider drag → keep the stored colour + swatch in sync.
void CVertEditDlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
{
    VED_SyncFromSliders();
    CWnd::OnHScroll( nSBCode, nPos, pScrollBar );
}

// [Color...] — the binary's sub_461480: pop a CColorDialog, on OK take its RGB into the
// stored colour (the binary then recomputes HSV + applies; we set RGB + the sliders and
// leave Apply explicit).  Alpha is unaffected by the colour picker (it has no alpha).
void CVertEditDlg::OnColorButton()
{
    CColorDialog dlg( RGB( s_vedColR, s_vedColG, s_vedColB ), 0, this );
    if ( dlg.DoModal() != IDOK )
        return;
    COLORREF c = dlg.GetColor();
    // Extract RGB bytes manually (the GetRValue/GetGValue/GetBValue macros clash with a
    // DXSDK redefinition under this target — they expand with an l-value-requiring '&').
    s_vedColR = (unsigned char)( c & 0xFF );
    s_vedColG = (unsigned char)( ( c >> 8 ) & 0xFF );
    s_vedColB = (unsigned char)( ( c >> 16 ) & 0xFF );
    VED_PushToSliders();
}

// [Apply] — read the enable checkboxes + the (synced) stored colour, run the apply core.
void CVertEditDlg::OnApply()
{
    VED_SyncFromSliders();
    bool doColour = ( s_vedChkClr && ::SendMessageA( s_vedChkClr, BM_GETCHECK, 0, 0 ) == BST_CHECKED );
    bool doAlpha  = ( s_vedChkAlp && ::SendMessageA( s_vedChkAlp, BM_GETCHECK, 0, 0 ) == BST_CHECKED );
    VertEditDlg_Apply( s_vedColR, s_vedColG, s_vedColB, s_vedColA, doColour, doAlpha );
    g_nUpdateBits |= 1;
}

void CVertEditDlg::OnClose()
{
    // Modeless: hide rather than destroy (OnVertexEditDlg toggles visibility).
    ShowWindow( SW_HIDE );
}

void CVertEditDlg::PostNcDestroy()
{
    g_dlgVertEdit = nullptr;
    s_vedR = s_vedG = s_vedB = s_vedA = nullptr;
    s_vedChkClr = s_vedChkAlp = s_vedSwatch = nullptr;
    delete this;
}

// 0x42BCD0  CMainFrame::OnVertexEditDlg toggles this; first call creates it.
void CVertEditDlg::Toggle()
{
    if ( g_dlgVertEdit && ::IsWindow( g_dlgVertEdit->GetSafeHwnd() ) )
    {
        BOOL vis = g_dlgVertEdit->IsWindowVisible();
        g_dlgVertEdit->ShowWindow( vis ? SW_HIDE : SW_SHOW );
        if ( !vis )
            g_dlgVertEdit->SetForegroundWindow();
        g_nUpdateBits |= 1;
        return;
    }

    CWnd *parent = AfxGetMainWnd();
    g_dlgVertEdit = new CVertEditDlg();

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 240, py = 180;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 160;  py = r.top + 120;
    }
    CRect rc( px, py, px + 330, py + 230 );
    if ( !g_dlgVertEdit->CreateEx( exStyle,
             AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                 ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
             "Vertex Edit", style, rc, parent, 0 ) )
    {
        delete g_dlgVertEdit;
        g_dlgVertEdit = nullptr;
        return;
    }
    g_nUpdateBits |= 1;
}

