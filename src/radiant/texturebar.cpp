#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Docked texture-alignment bar. It reads and writes the selected brush definition's
// current-layer texdef; spin controls reuse the selection texture-edit operations.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <stdio.h>
#include <stdlib.h>
// (CSpinButtonCtrl / UDN_DELTAPOS / NMUPDOWN come from <afxcmn.h> in the PCH.)

// ── selection state (select.cpp) ──────────────────────────────────────────────
#define SEL_FACE_COUNT() (g_SelectedFaces.GetSize())

// ── the quick texture ops (select.cpp, already ported — the SAME path the Surface
//    Inspector spinners use; the texture bar must NOT re-port these) ────────────
extern void Brush_ShiftTexture ( float a1, float a2 );   // 0x491F20
extern void Brush_ScaleTexture ( int   a1, int   a2 );   // 0x492650
extern void Brush_RotateTexture( int   a1 );             // 0x4929F0

// ── texdef target resolution + name (materialdef.cpp / surfacedlg.cpp) ─────────
extern LayerMaterialDef *Materialdef_GetName( MaterialDef *m );             // 0x431640
namespace LayerMat { int GetCurrentLayer( MaterialDef *def ); }             // 0x431B30

extern int          g_nUpdateBits;                    // 0x25D5A74 (engine_stubs.cpp)

// The radiant verbose Assert stub (engine_stubs.cpp): type 0 = log-to-stderr + CONTINUE.
extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ══════════════════════════════════════════════════════════════════════════════
//  texdef CORE — the picked face's MaterialDef (else the current-texture template).
//
//  Faithful to GetSurfaceAttributes' two branches: a selected face → its
//  brush->def->faces[index].mtldef[current_edit_layer]; otherwise the editor's
//  current-texture-window template random_texture_stuff[2100*layer].  Returns the
//  current-layer texdef_sub_t the bar reads/writes (NULL only if the def path is broken).
// ══════════════════════════════════════════════════════════════════════════════
// A MaterialDef is usable by the accessors (GetCurrentLayer / Materialdef_GetName) only when
// exactly one of lyrMtl/radMtl is set (the binary's MtlDef_IsValid invariant; an invalid def
// trips an Assert and then NULL-derefs radMtl->name).  Before the editor's current-texture
// template is initialised (e.g. a refresh that lands before the first map/material load), the
// template MaterialDef is zeroed — so guard it.  The binary's bar only refreshes AFTER init,
// where the template is always valid; this guard makes the early/no-texture path safe.
static inline bool TexBar_MtlDefUsable( const MaterialDef *m )
{
    return m && ( ( m->lyrMtl != nullptr ) ^ ( m->radMtl != nullptr ) );
}

// The picked face's MaterialDef (else the current-texture template).  NULL if no editable
// def is available (no selection + uninitialised template).
static MaterialDef *TexBar_TargetMaterialDef()
{
    MaterialDef *md;
    if ( SEL_FACE_COUNT() > 0 )
    {
        selface_t  &selFace = g_SelectedFaces.GetAt( 0 );
        selbrush_t *b = selFace.brush;
        if ( !b || !b->def )
            return nullptr;
        // TextureBar.cpp:160/161 (both type-0): the picked face must still be live + the
        // instance/def versions in sync.  The binary asserts these INLINE in BOTH
        // GetSurfaceAttributes (160/161) and ApplyFields (188/189, identical conditions);
        // this shared helper reports 160/161 for both — accepted file/line divergence.
        // (The texDef-null Assert 169 is a never-fire address-null check subsumed by the
        // upstream-null early-return.)
        iassert( selFace.face == &selFace.brush->faces[selFace.index] );    // TextureBar.cpp:160
        iassert( selFace.brush->version == selFace.brush->def->version );   // TextureBar.cpp:161
        md = &b->def->faces[selFace.index].mtldef[g_qeglobals.current_edit_layer];
    }
    else
    {
        md = &g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].mtl;
    }
    return TexBar_MtlDefUsable( md ) ? md : nullptr;
}

// The current-layer texdef_sub_t the bar reads/writes.  NULL when no usable MaterialDef.
static texdef_sub_t *TexBar_TargetTexdef()
{
    MaterialDef *md = TexBar_TargetMaterialDef();
    if ( !md )
        return nullptr;
    int layer = LayerMat::GetCurrentLayer( md );   // safe: md is MtlDef_IsValid here
    return &md->mat_texDef + layer;
}

// ══════════════════════════════════════════════════════════════════════════════
//  CTextureBar — the hand-built docked bar (CVehicleDlg / CSurfaceDlg plumbing pattern).
// ══════════════════════════════════════════════════════════════════════════════

// Control ids for the hand-built bar (the binary's are dialog-template resource ids; ours
// are a private contiguous block, matched to the spin→field routing in OnDeltaPos).
enum
{
    IDC_TB_TEXNAME = 1810,     // current texture name (read-only)
    IDC_TB_SHIFT_H, IDC_TB_SHIFT_H_SPIN,
    IDC_TB_SHIFT_V, IDC_TB_SHIFT_V_SPIN,
    IDC_TB_SCALE_H, IDC_TB_SCALE_H_SPIN,
    IDC_TB_SCALE_V, IDC_TB_SCALE_V_SPIN,
    IDC_TB_ROTATE,  IDC_TB_ROTATE_SPIN,
};

static HFONT s_tbFont = nullptr;

static HWND s_tbName    = nullptr;
static HWND s_tbShiftH  = nullptr, s_tbShiftHspin = nullptr;
static HWND s_tbShiftV  = nullptr, s_tbShiftVspin = nullptr;
static HWND s_tbScaleH  = nullptr, s_tbScaleHspin = nullptr;
static HWND s_tbScaleV  = nullptr, s_tbScaleVspin = nullptr;
static HWND s_tbRotate  = nullptr, s_tbRotateSpin = nullptr;
static bool s_tbRefreshing = false;   // guard: SetWindowText during a refresh must not re-apply

// CTextureBar member ints (the binary lays them out at +0x248..; here they are real fields,
// but the read/write helpers below transcribe the exact field→member mapping from the disasm).
//   m_nHShift  (0x248)   m_nHScale (0x24C)   m_nRotate (0x250)
//   m_nVShift  (0x254)   m_nVScale (0x258)   m_nRotateAmt (0x25C, init 45)

static int g_tbRotateAmt = 45;        // CTextureBar member @0x25C (the Rotate spin step)

static HWND TB_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                          int x, int y, int w, int hgt, const char *text = nullptr )
{
    HWND h = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                              x, y, w, hgt, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_tbFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_tbFont, 0 );
    return h;
}

BEGIN_MESSAGE_MAP( CTextureBar, CWnd )
    ON_WM_CREATE()
    ON_NOTIFY( UDN_DELTAPOS, IDC_TB_SHIFT_H_SPIN, &CTextureBar::OnDeltaPos )
    ON_NOTIFY( UDN_DELTAPOS, IDC_TB_SHIFT_V_SPIN, &CTextureBar::OnDeltaPos )
    ON_NOTIFY( UDN_DELTAPOS, IDC_TB_SCALE_H_SPIN, &CTextureBar::OnDeltaPos )
    ON_NOTIFY( UDN_DELTAPOS, IDC_TB_SCALE_V_SPIN, &CTextureBar::OnDeltaPos )
    ON_NOTIFY( UDN_DELTAPOS, IDC_TB_ROTATE_SPIN,  &CTextureBar::OnDeltaPos )
    ON_EN_KILLFOCUS( IDC_TB_SHIFT_H, &CTextureBar::OnFieldEdited )
    ON_EN_KILLFOCUS( IDC_TB_SHIFT_V, &CTextureBar::OnFieldEdited )
    ON_EN_KILLFOCUS( IDC_TB_SCALE_H, &CTextureBar::OnFieldEdited )
    ON_EN_KILLFOCUS( IDC_TB_SCALE_V, &CTextureBar::OnFieldEdited )
    ON_EN_KILLFOCUS( IDC_TB_ROTATE,  &CTextureBar::OnFieldEdited )
END_MESSAGE_MAP()

CTextureBar::CTextureBar()
{
}

// One labelled "edit + up-down spinner" cell laid out horizontally across the bar.
static void TB_MakeCell( HWND self, int &x, int y, const char *label, int lblW,
                         int editId, HWND *editOut, int spinId, HWND *spinOut )
{
    const int editW = 52, spinW = 15, h = 20;
    TB_MakeChild( self, "static", SS_LEFT, 0, x, y + 3, lblW, 16, label );
    x += lblW + 2;
    HWND e = TB_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, editId, x, y, editW, h );
    x += editW;
    // A bare up-down (no buddy auto-int): we read iDelta in UDN_DELTAPOS and forward the
    // ±gridsize nudge to the ported Brush_*Texture op ourselves.
    HWND s = TB_MakeChild( self, UPDOWN_CLASSA, UDS_ARROWKEYS, spinId, x, y, spinW, h );
    x += spinW + 10;
    if ( editOut ) *editOut = e;
    if ( spinOut ) *spinOut = s;
}

// 0x459250  CTextureBar::Init — the ctor.  Here: build the child controls across the bar.
int CTextureBar::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_tbFont )
        s_tbFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    INITCOMMONCONTROLSEX icc = { sizeof( icc ), ICC_UPDOWN_CLASS };
    InitCommonControlsEx( &icc );

    g_tbRotateAmt = 45;     // member @0x25C init (CTextureBar::Init sets [esi+25Ch]=45)

    HWND self = GetSafeHwnd();
    int x = 6, y = 4;

    TB_MakeChild( self, "static", SS_LEFT, 0, x, y + 3, 30, 16, "Tex" );
    x += 32;
    s_tbName = TB_MakeChild( self, "edit", WS_BORDER | ES_READONLY | ES_AUTOHSCROLL,
                             IDC_TB_TEXNAME, x, y, 150, 20 );
    x += 160;

    TB_MakeCell( self, x, y, "SH", 22, IDC_TB_SHIFT_H, &s_tbShiftH, IDC_TB_SHIFT_H_SPIN, &s_tbShiftHspin );
    TB_MakeCell( self, x, y, "SV", 22, IDC_TB_SHIFT_V, &s_tbShiftV, IDC_TB_SHIFT_V_SPIN, &s_tbShiftVspin );
    TB_MakeCell( self, x, y, "ScH", 26, IDC_TB_SCALE_H, &s_tbScaleH, IDC_TB_SCALE_H_SPIN, &s_tbScaleHspin );
    TB_MakeCell( self, x, y, "ScV", 26, IDC_TB_SCALE_V, &s_tbScaleV, IDC_TB_SCALE_V_SPIN, &s_tbScaleVspin );
    TB_MakeCell( self, x, y, "Rot", 26, IDC_TB_ROTATE, &s_tbRotate, IDC_TB_ROTATE_SPIN, &s_tbRotateSpin );

    // No initial GetSurfaceAttributes here: the bar is created during CMainFrame::OnCreate,
    // before the current-texture template / a map is up.  The first real refresh comes from
    // CMainFrame::UpdateWindows / the post-load broadcast (faithful to the binary, which
    // refreshes the bar via UpdateTextureBar after init, never from the ctor).
    return 0;
}

// ── format / read helpers (the binary's DDX_Text int<->edit; we keep them %d, the bar
//    shows integer texels/degrees exactly like the IDB member ints) ─────────────
static void TB_SetInt( HWND h, int v )
{
    if ( !h ) return;
    char buf[32];
    sprintf( buf, "%d", v );
    ::SetWindowTextA( h, buf );
}

static int TB_GetInt( HWND h )
{
    if ( !h ) return 0;
    char buf[32] = { 0 };
    ::GetWindowTextA( h, buf, sizeof( buf ) - 1 );
    return atoi( buf );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x459610  CTextureBar::GetSurfaceAttributes — READ the picked face's texdef into the
// member ints and DISPLAY them (UpdateData(FALSE)).  Faithful field mapping (disasm):
//   shift[0] → m_nHShift   shift[1] → m_nVShift
//   size[0]  → m_nHScale   size[1]  → m_nVScale   rotate → m_nRotate
// Each is int(float) (truncating __ftol2).  The bar shows raw texel scale/shift + degrees
// (NOT the Surface Inspector's size/width "stretch" — the texture bar is the raw-texel view).
// THE REFRESH ENTRY POINT (called whenever the current texture / selection changes).
// ─────────────────────────────────────────────────────────────────────────────
void CTextureBar::GetSurfaceAttributes( CTextureBar *bar )
{
    if ( !bar || !bar->GetSafeHwnd() || !s_tbShiftH )
        return;

    texdef_sub_t *td = TexBar_TargetTexdef();
    if ( !td )
        return;

    s_tbRefreshing = true;

    // Texture name (the bar's current-material readout) — the brief's "shows the current
    // material name".  TexBar_TargetMaterialDef only returns a MtlDef_IsValid def, so
    // Materialdef_GetName is safe (no NULL radMtl->name deref).
    MaterialDef *md = TexBar_TargetMaterialDef();
    const char *name = md ? (const char *)Materialdef_GetName( md ) : "";
    ::SetWindowTextA( s_tbName, name ? name : "" );

    TB_SetInt( s_tbShiftH, (int)td->shift[0] );   // m_nHShift @0x248
    TB_SetInt( s_tbShiftV, (int)td->shift[1] );   // m_nVShift @0x254
    TB_SetInt( s_tbScaleH, (int)td->size[0] );    // m_nHScale @0x24C
    TB_SetInt( s_tbScaleV, (int)td->size[1] );    // m_nVScale @0x258
    TB_SetInt( s_tbRotate, (int)td->rotate );     // m_nRotate @0x250

    s_tbRefreshing = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x459750  TextureBar_02 (via the 0x459600 thunk) — UpdateData(TRUE) then WRITE the
// member ints back onto the picked face's texdef (live edit-as-you-type), g_nUpdateBits|=1.
// No Brush_SetTexture propagation — the binary writes the picked face's def directly and
// relies on the view invalidation to re-read (the spin nudges below DO propagate, via the
// already-ported Brush_*Texture).  Faithful field mapping (disasm v5[2]/v5[3]/v5[0]/v5[1]/v5[4]):
//   m_nHShift → shift[0]   m_nVShift → shift[1]
//   m_nHScale → size[0]    m_nVScale → size[1]   m_nRotate → rotate
// Each member is read as int and stored as (float).  No-op when nothing is selected
// (result == GetSelectedFaces size == 0).
// ─────────────────────────────────────────────────────────────────────────────
void CTextureBar::ApplyFields()
{
    if ( s_tbRefreshing )
        return;
    if ( SEL_FACE_COUNT() <= 0 )
        return;                       // the binary's `if (GetSelectedFaces.size)` guard

    texdef_sub_t *td = TexBar_TargetTexdef();
    if ( !td )
        return;

    td->shift[0] = (float)TB_GetInt( s_tbShiftH );   // m_nHShift → shift[0]
    td->shift[1] = (float)TB_GetInt( s_tbShiftV );   // m_nVShift → shift[1]
    td->size[0]  = (float)TB_GetInt( s_tbScaleH );   // m_nHScale → size[0]
    td->size[1]  = (float)TB_GetInt( s_tbScaleV );   // m_nVScale → size[1]
    td->rotate   = (float)TB_GetInt( s_tbRotate );   // m_nRotate → rotate

    g_nUpdateBits |= 1;
}

void CTextureBar::OnFieldEdited()
{
    ApplyFields();
}

// ── the five spin handlers (sub_459470/4594C0/459510/459550/459590) ───────────
// HShift/VShift spins: ±gridsize via Brush_ShiftTexture; HScale/VScale spins: ±gridsize via
// Brush_ScaleTexture; Rotate spin: ±m_nRotateAmt via Brush_RotateTexture.  Each then refreshes
// the member ints via GetSurfaceAttributes (the binary's tail).  `iDelta>=0` (down arrow) =
// negative step (disasm `if (*(int*)(a2+16) >= 0) v4 = -v4`).
void CTextureBar::OnDeltaPos( NMHDR *pNMHDR, LRESULT *pResult )
{
    NMUPDOWN *ud = (NMUPDOWN *)pNMHDR;
    bool negate  = ( ud->iDelta >= 0 );    // faithful: positive delta negates the step

    // disasm 0x45947e: `mov eax,[d_savedinfo.d_gridsize]; cdq;xor;sub` (INTEGER abs) then `fild`
    // (INT->float) — the binary stores d_savedinfo.d_gridsize @0x2BC as an INT (the texel snap is
    // integer); the port typed it float (qe3.h:640) so (int)8.0f == fild(int 8) for every grid the
    // texel shift uses.  (Only texturebar + mainfrm read this float field, both (int)-cast it.)
    int grid = (int)g_qeglobals.d_savedinfo.d_gridsize;
    if ( grid < 0 ) grid = -grid;          // abs32(d_gridsize)
    int step = negate ? -grid : grid;

    switch ( (int)pNMHDR->idFrom )
    {
        case IDC_TB_SHIFT_H_SPIN:  Brush_ShiftTexture( (float)step, 0.0f ); break;   // sub_459470
        case IDC_TB_SHIFT_V_SPIN:  Brush_ShiftTexture( 0.0f, (float)step ); break;   // sub_4594C0
        case IDC_TB_SCALE_H_SPIN:  Brush_ScaleTexture( step, 0 );           break;   // sub_459510
        case IDC_TB_SCALE_V_SPIN:  Brush_ScaleTexture( 0, step );           break;   // sub_459550
        case IDC_TB_ROTATE_SPIN:                                                     // sub_459590
        {
            int rot = g_tbRotateAmt;       // member @0x25C (UpdateData(TRUE) reads it first)
            if ( rot < 0 ) rot = -rot;
            Brush_RotateTexture( negate ? -rot : rot );
            break;
        }
        default:
            if ( pResult ) *pResult = 0;
            return;
    }

    GetSurfaceAttributes( this );          // refresh the member ints (disasm tail)
    if ( pResult )
        *pResult = 0;
}

// ══════════════════════════════════════════════════════════════════════════════
//  CMainFrame integration — the bar is created across the top strip of the frame.
//  CMainFrame::UpdateTextureBar (0x429150) refreshes it; the QE4 layout insets below it.
// ══════════════════════════════════════════════════════════════════════════════
static CTextureBar *g_pTextureBarWnd = nullptr;   // points at CMainFrame::m_wndTextureBar
int g_texBarHeight = 0;                            // top inset consumed by the bar (0 = none)

// Create the embedded texture bar across the top of the frame.  Returns its height (so the
// caller can inset the QE4 layout).  Hand-built (radiant.rc has no template) — a child window
// of the frame holding the name static + the five edit/spin cells.
int CTextureBar::CreateBar( CWnd *frame, CTextureBar *bar, int frameWidth )
{
    if ( !frame || !bar )
        return 0;
    const int barH = 28;
    if ( !bar->CreateEx( 0,
             AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW, ::LoadCursor( NULL, IDC_ARROW ),
                                  (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
             "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
             CRect( 0, 0, frameWidth, barH ), frame, 0 ) )
        return 0;
    g_pTextureBarWnd = bar;
    g_texBarHeight   = barH;
    return barH;
}

// Reposition the bar across the top when the frame resizes.
void CTextureBar::LayoutBar( int frameWidth )
{
    if ( g_pTextureBarWnd && g_pTextureBarWnd->GetSafeHwnd() && g_texBarHeight > 0 )
        g_pTextureBarWnd->MoveWindow( 0, 0, frameWidth, g_texBarHeight );
}

