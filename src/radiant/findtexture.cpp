#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Find/replace textures across brushes, patches, and optional prefab sub-maps.
// Hex-Rays invents FPU arguments for 0x493160; its real signature is the one below.
// sub_492E20 receives a 56-byte selbrush_t instance, not the 88-byte brush definition.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <string>

extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ── prefab-recursion dependencies (map.cpp) ───────────────────────────────────
// The (owner->prefab && flag&4) branch enters the selected prefab's referenced .map,
// re-runs the find/replace inside it, and re-saves it to disk — the real editor's
// "recurse into prefabs" behaviour (sub_492E20 0x492E5D..0x492F63).
extern void Prefab_NextLevel( void *a1 );   // map.cpp 0x489190 (ENTER the prefab sub-map)
extern void Prefab_PrevLevel();             // map.cpp 0x489890 (LEAVE, restore parent)
extern void Map_SaveFile( const char *path, char a1, char a2 );  // map.cpp 0x486C00
extern char currentmap[];                   // map.cpp 0x23F18D8 (the loaded prefab .map path)

// ══════════════════════════════════════════════════════════════════════════════
//  g_findReplaceVisited  —  std::set<std::string>  (IDB head/node @0x26656B0/B4).
//
//  The prefab-recursion branch dedups the prefab .map names it has already entered so
//  a diamond/self prefab reference can't recurse forever.  The IDB's container is a
//  textbook MSVC std::set<std::string> RB-tree:
//    * FindReplaceVisited_Find   (sub_4944B0) = lower_bound (sub_494540) + a !(key<*it)
//      ordering check (sub_413C70 = std::string operator<) → set::find.
//    * FindReplaceVisited_Insert (sub_41E590) = _Tree::insert returning pair<it,bool>.
//    * Str_ConstructFromCStr     (0x412760)   = std::string(const char*).
//  Per the #18 STL-collapse rule (the editor owns reader+writer; no cross-module ABI),
//  the faithful translation is an ordinary std::set<std::string> with set::find /
//  set::insert.  The visited set is RESET (Set_EraseTreeRec + reset the 3 RB sentinels,
//  = clear()) at each top-level entry point — the dialog OnOK/OnApply handlers
//  (0x415B50 / 0x415A90) AND LayeredMaterials_texcoords (0x417190) — NOT inside
//  FindReplaceTextures, so the recursion's re-entry of FindReplaceTextures accumulates
//  into the same set rather than wiping it mid-walk.
static std::set<std::string> g_findReplaceVisited;

// Reset the prefab-recursion visited set (IDB: Set_EraseTreeRec(...right) + reset the 3
// RB-tree sentinels + dword_26656B8=0 → clear()).  Called by the dialog handlers and by
// LayeredMaterials_texcoords before each top-level FindReplaceTextures invocation.
void FindReplaceVisited_Reset()
{
    g_findReplaceVisited.clear();
}

// ── brush lists + selection (engine_stubs / map.cpp / select.cpp) ──────────────
extern selbrush_t selected_brushes;          // 0x23F1864
extern selbrush_t active_brushes;            // 0x23F189C
extern void       Select_Deselect( int keepSelected );        // select.cpp 0x48E800
extern int        g_nUpdateBits;             // engine_stubs 0x25D5A74

// ── material name resolution + apply primitives ───────────────────────────────
// NOTE: the IDB inlines LayeredMaterials_GetMaterial(name) / else Texture_GetHandle(name)
// to resolve the replacement material; we call SetMaterial (which IS that dispatch, but
// with the headless-safe degenerate fallback — Texture_GetHandle needs the live renderer).
extern void              SetMaterial( const char *name, patchMesh_material *out );  // materialdef.cpp 0x4315C0
extern LayerMaterialDef *Materialdef_GetName( MaterialDef *m );                     // materialdef.cpp 0x431640
extern int               Init_MaterialLayer( MaterialDef *channel, MaterialDef *src ); // materialdef.cpp 0x472C00
namespace LayerMat { int GetCurrentLayer( MaterialDef *def ); }                     // 0x431B30

// face texdef apply (brush.cpp):
//   Brush_SetFaceTexdefSize (0x476740) — write the {Material*, qtexture*} pair into the
//     face's current-layer slot (the IDB's sub_476740 per-face apply, a1 = a 2-elem array).
//   Brush_SetFaceTexdef     (0x4767e0) — copy a full texdef_sub_t mapping into the slot.
extern void    Brush_SetFaceTexdefSize( const float *size2, face_t *f, brush_t *b );  // 0x476740
extern void    sub_4767E0( const texdef_sub_t *texDef, int facePtr, int brushDef );   // 0x4767E0 Brush_SetFaceTexdef

// rebuild / housekeeping (brush.cpp / select.cpp / map.cpp):
extern void    Brush_BuildWindings( brush_t *b, int bFull );  // 0x477AC0
extern void    SetupVertexSelection();                        // 0x494BC0
extern void    MarkMapModified();                             // 0x499BB0

// ── current-texture-window MaterialDef (qeglobals_t.random_texture_stuff) ──────
// The IDB indexes &random_texture_stuff[2100 * current_edit_layer] as the active
// MaterialDef, and [...+36] (= mat_texDef.sample_size, byte 36) as a packed sample-size
// float (the flag&8 "Live" path seeds the layer mapping from it).
//   2100 = 0x834 = the per-layer stride (a LayerMat block); the MaterialDef is at its head.

// ══════════════════════════════════════════════════════════════════════════════
//  Patch_FindReplaceTexture  (0x449520) — patch (pmesh) face material swap.
//  a1 = the patch brush DEF; replaceName / findName; flags.  When (flags&2)==0 the
//  patch's current-layer material name must equal findName.  On a match → SetMaterial
//  (replaceName) into &patch->texture[layer] + ++version.  Returns 1 if it replaced.
// ══════════════════════════════════════════════════════════════════════════════
char Patch_FindReplaceTexture( brush_t *a1, const char *replaceName,
                               const char *findName, char flags )
{
    if ( !a1 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\PMESH.CPP", 7120, 0, "%s", "b" );
    if ( !a1->patch )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\PMESH.CPP", 7121, 0, "%s", "b->patch" );

    patchMesh_t *patch = a1->patch;
    patchMesh_material *slot = &patch->texture + g_qeglobals.current_edit_layer;
    if ( ( flags & 2 ) == 0 )
    {
        const char *name = (const char *)Materialdef_GetName( (MaterialDef *)slot );
        if ( _stricmp( name, findName ) )
            return 0;                       // current name != find → skip
    }
    SetMaterial( replaceName, slot );
    ++patch->version;
    return 1;
}

// ══════════════════════════════════════════════════════════════════════════════
//  FindReplaceTexture_Brush  (sub_492E20) — per-brush find/replace worker.
//  `inst` is the selbrush_t instance, not its brush definition.
//  Returns 1 if any face/patch was replaced on this brush.
// ══════════════════════════════════════════════════════════════════════════════
// Forward decl (the prefab branch recurses through FindReplaceTextures).
bool FindReplaceTextures( const char *find, const char *replace, char flags );

static char FindReplaceTexture_Brush( selbrush_t *inst, const char *findName,
                                      const char *replaceName, char flags )
{
    entity_s *owner = inst->owner;
    if ( owner->prefab )
    {
        // flag&4 edits and re-saves referenced prefab maps. The visited set prevents cycles;
        // bit 0 is cleared so recursion walks the whole sub-map.
        char prefabReplaced = 0;
        if ( ( flags & 4 ) != 0 )
        {
            // Prefab .map name = owner->def->modelClass->x02 ([modelClass + 4]).
            entity_s_def *def = (entity_s_def *)owner->def;
            const char   *prefabMap = *(const char **)( (char *)def->modelClass + 4 );

            // g_findReplaceVisited.find(prefabMap) == end() → not yet visited.
            if ( g_findReplaceVisited.find( prefabMap ) == g_findReplaceVisited.end() )
            {
                g_findReplaceVisited.insert( prefabMap );      // mark visited (set::insert)

                Prefab_NextLevel( inst );                      // ENTER the prefab sub-map
                // flags & 0xFE: clear bit0 ("selected only") so the recursion walks ALL of
                // the sub-map's brushes; keep bits 2/4/8 so nested prefabs recurse too.
                prefabReplaced = FindReplaceTextures( findName, replaceName,
                                                      (char)( flags & 0xFE ) ) ? 1 : 0;
                if ( prefabReplaced )
                    Map_SaveFile( currentmap, 0, 0 );          // re-save the modified .map
                Prefab_PrevLevel();                            // LEAVE, restore the parent
            }
        }
        return prefabReplaced;               // prefab owner: never falls through to the
                                             // face/patch walk (disasm jumps to the epilogue)
    }

    char replaced = 0;

    // Patch path: inst->patch (hex-rays "mins[0]") non-null → it's a patch brush.
    if ( inst->patch )
    {
        if ( Patch_FindReplaceTexture( inst->def, replaceName, findName, flags ) )
            replaced = 1;
    }

    // Face walk over the brush DEF's faces.  The IDB reads inst->faceCount (hex-rays
    // "unk1", the 56-byte INSTANCE field @0x18) — but that is the LAZY faceVis cache that
    // sub_477D70 syncs from def->faceCount only when the brush is drawn/selected (0 on a
    // freshly-loaded, undrawn brush — the recurring instance-vs-def trap).  In the GUI the
    // dialog always runs after a draw so it is synced; we read def->faceCount directly (the
    // authoritative count the apply primitives assert against — Brush_SetFaceTexdefSize uses
    // &b->faces[b->faceCount]), so the replace also works headless / pre-draw.
    brush_t *def = inst->def;
    if ( def && def->faceCount )
    {
        face_t  *faces = def->faces;
        for ( int fi = 0; fi < (int)(unsigned)def->faceCount; ++fi, ++faces )
        {
            MaterialDef *md = &faces->mtldef[g_qeglobals.current_edit_layer];

            bool match;
            if ( flags & 2 )
            {
                match = true;                // "replace everywhere, don't test against Find"
            }
            else
            {
                // MtlDef_IsValid + name extraction (sub_492E20 inlined): exactly one of
                // lyrMtl / radMtl; name = (char*)lyrMtl, or radMtl->name (qtexture_s+4).
                if ( !md || ( (int)( md->lyrMtl != nullptr ) + (int)( md->radMtl != nullptr ) ) != 1 )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                            85, 0, "%s", "MtlDef_IsValid( mtlDef )" );
                const char *name = (const char *)md->lyrMtl;
                if ( !md->lyrMtl )
                    name = md->radMtl->name;
                match = ( _stricmp( name, findName ) == 0 );
            }

            if ( !match )
                continue;

            // flag&8 "Live": also stamp the current-texture-window's layer MAPPING onto
            // this face (Init_MaterialLayer reseeds the active layer from its sample-size,
            // then Brush_SetFaceTexdef copies that texdef in). The sample-size
            // is the float bit-pattern at random_texture_stuff[2100*layer + 36].
            if ( ( flags & 8 ) != 0 )
            {
                int            layer  = g_qeglobals.current_edit_layer;
                MaterialDef   *curMtl = &g_qeglobals.random_texture_stuff[layer].mtl;
                MaterialDef   *sampleArg;
                memcpy( &sampleArg, &g_qeglobals.random_texture_stuff[layer].sampleSize, sizeof( sampleArg ) );
                Init_MaterialLayer( curMtl, sampleArg );
                int curLayer = LayerMat::GetCurrentLayer( curMtl );
                sub_4767E0( &curMtl->mat_texDef + curLayer,
                            (int)(intptr_t)faces, (int)(intptr_t)def );
            }

            // Resolve the replacement name to its {lyrMtl, radMtl} pair and write it into
            // the face's current-layer slot (the proven texture-apply path: the IDB's
            // sub_476740 / Brush_SetFaceTexdefSize).  The IDB inlines
            // LayeredMaterials_GetMaterial(name) / else Texture_GetHandle(name) here; we go
            // through SetMaterial (materialdef.cpp) instead — it is EXACTLY that dispatch
            // but with the headless-safe degenerate fallback when the renderer/material
            // system isn't up (Texture_GetHandle needs the renderer; SetMaterial gates on
            // g_radiantFirstLightRendererReady).  This is the same call the texwnd
            // click-apply (proven) uses.
            patchMesh_material pair{};
            SetMaterial( replaceName, &pair );
            if ( ( (int)( pair.lyrMtl != nullptr ) + (int)( pair.radMtl != nullptr ) ) != 1 )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                        65, 0, "%s", "MtlDef_IsValid( mtlDef )" );
            Brush_SetFaceTexdefSize( (const float *)&pair, faces, def );
            replaced = 1;
        }
    }

    // Rebuild the brush after edits (matches the IDB tail: build windings, vertex-sel
    // upkeep, mark modified, bump version).  (def resolved above; always valid for a real
    // brush — guarded so a degenerate node can't AV.)
    if ( def )
    {
        Brush_BuildWindings( def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++def->version;
    }
    g_nUpdateBits |= 1u;

    return replaced;
}

// ══════════════════════════════════════════════════════════════════════════════
//  FindReplaceTextures  (0x493160) — the public entry.  flag&1 = selected brushes
//  only (else deselect everything and walk active_brushes).  Returns true if any
//  face was replaced.
// ══════════════════════════════════════════════════════════════════════════════
bool FindReplaceTextures( const char *find, const char *replace, char flags )
{
    selbrush_t *list = &selected_brushes;
    if ( ( flags & 1 ) == 0 )
    {
        list = &active_brushes;
        Select_Deselect( 1 );
    }

    bool replaced = false;
    for ( selbrush_t *i = list->next; i != list; i = i->next )
    {
        if ( FindReplaceTexture_Brush( i, find, replace, flags ) )
            replaced = true;
    }

    g_nUpdateBits |= 1u;
    return replaced;
}

// ══════════════════════════════════════════════════════════════════════════════
// Find/replace texture modeless popup. The hand-built controls preserve the binary IDs
// and flag layout because the original dialog resource is unavailable.
// ══════════════════════════════════════════════════════════════════════════════

CFindTextureDlg *g_dlgFind = nullptr;   // the singleton instance (NULL until first opened)

enum
{
    IDC_FR_FIND = 1097,        // find material (matches the IDB control IDs)
    IDC_FR_REPLACE = 1101,
    IDC_FR_SELONLY = 1105,     // "Use selected brushes only"           → flag&1
    IDC_FR_FORCE  = 1109,      // "Replace everywhere, don't test Find" → flag&2
    IDC_FR_RECURSE = 1525,     // "Recurse into prefabs"                → flag&4 (CoD extra)
    IDC_FR_LIVE   = 1684,      // "Live: also copy the current texture mapping" → flag&8
    IDC_FR_LBL_FIND = 1990,
    IDC_FR_LBL_REPLACE,
    IDC_FR_BTN_OK = 1,         // IDOK
    IDC_FR_BTN_APPLY = 1995,
    IDC_FR_BTN_CLOSE = 2,      // IDCANCEL
};

static HFONT s_frFont    = nullptr;
static HWND  s_frFind    = nullptr;
static HWND  s_frReplace = nullptr;
static HWND  s_frSelOnly = nullptr;
static HWND  s_frForce   = nullptr;
static HWND  s_frRecurse = nullptr;
static HWND  s_frLive    = nullptr;

extern BOOL SaveRegistryInfo( const char *pszName, void *pvBuf, int lSize );
extern BOOL LoadRegistryInfo( const char *pszName, void *pvBuf, long *plSize );

// byte_73C380 (0x73c380) — which edit field a picked texture name flows into: 1 = Find
// (sub_415CE0), 0 = Replace (sub_415D40).  The IDB toggles it from the per-edit
// EN_SETFOCUS handlers sub_415DF0 (Find 1097 → 1) / sub_415E00 (Replace 1101 → 0); BSS
// default 0 (Replace).  Read by Texture_SetTexture (texwnd.cpp 0x45be50).
char byte_73C380 = 0;

static HWND FR_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                          int x, int y, int w, int hgt, const char *text = nullptr )
{
    HWND h = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                              x, y, w, hgt, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_frFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_frFont, 0 );
    return h;
}

BEGIN_MESSAGE_MAP( CFindTextureDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_FR_BTN_OK,    &CFindTextureDlg::OnFindReplaceOK )
    ON_BN_CLICKED( IDC_FR_BTN_APPLY, &CFindTextureDlg::OnFindReplaceApply )
    ON_BN_CLICKED( IDC_FR_BTN_CLOSE, &CFindTextureDlg::OnFindReplaceClose )
    // EN_SETFOCUS on each edit drives byte_73C380 (which field a picked texture fills):
    // IDB message map (0x6d7900/0x6d7918): WM_COMMAND/EN_SETFOCUS, IDs 1097/1101.
    ON_EN_SETFOCUS( IDC_FR_FIND,    &CFindTextureDlg::OnFindSetFocus )     // sub_415DF0
    ON_EN_SETFOCUS( IDC_FR_REPLACE, &CFindTextureDlg::OnReplaceSetFocus )  // sub_415E00
END_MESSAGE_MAP()

CFindTextureDlg::CFindTextureDlg()
{
}

int CFindTextureDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_frFont )
        s_frFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 8, lblW = 56, editW = 200, row = 22;
    int y = 8;

    FR_MakeChild( self, "static", SS_LEFT, IDC_FR_LBL_FIND,    M, y + 3, lblW, row, "Find:" );
    s_frFind = FR_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_FR_FIND,
                             M + lblW, y, editW, row );
    y += 28;
    FR_MakeChild( self, "static", SS_LEFT, IDC_FR_LBL_REPLACE, M, y + 3, lblW, row, "Replace:" );
    s_frReplace = FR_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_FR_REPLACE,
                                M + lblW, y, editW, row );
    y += 32;

    s_frSelOnly = FR_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_FR_SELONLY,
                                M, y, editW + lblW, 18, "Use selected brushes only" ); y += 22;
    s_frForce   = FR_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_FR_FORCE,
                                M, y, editW + lblW, 18, "Replace everywhere (don't test against Find)" ); y += 22;
    s_frRecurse = FR_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_FR_RECURSE,
                                M, y, editW + lblW, 18, "Recurse into prefabs (re-saves referenced .map files)" ); y += 22;
    s_frLive    = FR_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_FR_LIVE,
                                M, y, editW + lblW, 18, "Live: also copy the current texture mapping" ); y += 26;
    // Default: Live ON (matches GtkRadiant m_bLive = TRUE), others off.
    ::SendMessageA( s_frLive, BM_SETCHECK, BST_CHECKED, 0 );

    FR_MakeChild( self, "button", BS_DEFPUSHBUTTON | WS_TABSTOP, IDC_FR_BTN_OK,    M,        y, 70, 24, "OK" );
    FR_MakeChild( self, "button", BS_PUSHBUTTON    | WS_TABSTOP, IDC_FR_BTN_APPLY, M + 78,   y, 70, 24, "Apply" );
    FR_MakeChild( self, "button", BS_PUSHBUTTON    | WS_TABSTOP, IDC_FR_BTN_CLOSE, M + 156,  y, 70, 24, "Close" );

    // Pre-fill Find with the current-texture-window material name (the active texture).
    int layer = g_qeglobals.current_edit_layer;
    if ( layer >= 0 && layer < 3 )
    {
        MaterialDef *cur = &g_qeglobals.random_texture_stuff[layer].mtl;
        if ( ( (cur->lyrMtl != nullptr) + (cur->radMtl != nullptr) ) == 1 )
        {
            const char *nm = (const char *)Materialdef_GetName( cur );
            if ( nm && nm[0] )
                ::SetWindowTextA( s_frFind, nm );
        }
    }
    return 0;
}

// Read the controls, assemble the flag byte, run FindReplaceTextures.
static void FR_DoReplace()
{
    if ( g_dlgFind && ::IsWindow( g_dlgFind->GetSafeHwnd() ) )
    {
        RECT rect;
        g_dlgFind->GetWindowRect( &rect );
        SaveRegistryInfo( "Radiant::TextureFindWindow", &rect, sizeof( rect ) );
    }

    char findBuf[256] = { 0 }, replBuf[256] = { 0 };
    if ( s_frFind )    ::GetWindowTextA( s_frFind,    findBuf, sizeof( findBuf ) - 1 );
    if ( s_frReplace ) ::GetWindowTextA( s_frReplace, replBuf, sizeof( replBuf ) - 1 );
    if ( !replBuf[0] )                 // nothing to replace with → no-op (GtkRadiant: FindReplace skips empty)
        return;

    // Flag byte (IDB OnOK/OnApply 0x415B50 / 0x415A90 — the DDX members at this+116/128/132/140):
    //   bit0 = selected-only, bit1 = force-replace-all, bit2 = recurse-prefabs, bit3 = live.
    char flags = 0;
    if ( s_frSelOnly && SendMessageA( s_frSelOnly, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) flags |= 1;
    if ( s_frForce   && SendMessageA( s_frForce,   BM_GETCHECK, 0, 0 ) == BST_CHECKED ) flags |= 2;
    if ( s_frRecurse && SendMessageA( s_frRecurse, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) flags |= 4;
    if ( s_frLive    && SendMessageA( s_frLive,    BM_GETCHECK, 0, 0 ) == BST_CHECKED ) flags |= 8;

    // Reset the prefab-recursion visited set at the top-level entry (the IDB's
    // Set_EraseTreeRec + reset-sentinels before each FindReplaceTextures call) so a
    // fresh OK/Apply starts with an empty visited set; the recursion accumulates into it.
    FindReplaceVisited_Reset();

    FindReplaceTextures( findBuf, replBuf, flags );
    g_nUpdateBits = -1;
}

void CFindTextureDlg::OnFindReplaceOK()
{
    FR_DoReplace();
    DestroyWindow();
}

void CFindTextureDlg::OnFindReplaceApply()
{
    FR_DoReplace();
}

void CFindTextureDlg::OnFindReplaceClose()
{
    DestroyWindow();
}

void CFindTextureDlg::OnClose()
{
    DestroyWindow();
}

void CFindTextureDlg::PostNcDestroy()
{
    g_dlgFind   = nullptr;
    s_frFind = s_frReplace = s_frSelOnly = s_frForce = s_frRecurse = s_frLive = nullptr;
    delete this;                 // modeless self-cleanup
}

// 0x415c20  CFindTextureDlg::show — open (or re-focus) the singleton.
void CFindTextureDlg::show()
{
    if ( g_dlgFind && ::IsWindow( g_dlgFind->GetSafeHwnd() ) )
    {
        g_dlgFind->ShowWindow( SW_SHOW );
        g_dlgFind->SetForegroundWindow();
        return;
    }

    CWnd *parent = AfxGetMainWnd();
    g_dlgFind = new CFindTextureDlg();

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 220, py = 220;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 160;  py = r.top + 140;
    }
    CRect rc( px, py, px + 290, py + 226 );
    if ( !g_dlgFind->CreateEx( exStyle, AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                                   ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
                               "Find / Replace Texture(s)", style, rc, parent, 0 ) )
    {
        delete g_dlgFind;
        g_dlgFind = nullptr;
        return;
    }

    RECT savedRect;
    long savedSize = sizeof( savedRect );
    if ( LoadRegistryInfo( "Radiant::TextureFindWindow", &savedRect, &savedSize ) )
    {
        g_dlgFind->SetWindowPos( nullptr, savedRect.left, savedRect.top, 0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER );
    }
}

// sub_415DF0 (0x415df0) / sub_415E00 (0x415e00) — EN_SETFOCUS handlers for the Find /
// Replace edits.  Each just records which field has focus so Texture_SetTexture knows
// which one to fill on a texture pick.  IDB bodies: `mov byte_73C380, 1; retn` and
// `mov byte_73C380, 0; retn`.
void CFindTextureDlg::OnFindSetFocus()    { byte_73C380 = 1; }   // 0x415df0
void CFindTextureDlg::OnReplaceSetFocus() { byte_73C380 = 0; }   // 0x415e00

// ══════════════════════════════════════════════════════════════════════════════
//  sub_415CE0 (0x415ce0)  /  sub_415D40 (0x415d40) — push a picked material name
//  into the dialog's Find / Replace text field.  Texture_SetTexture (texwnd.cpp
//  0x45be50) calls `byte_73C380 ? SetFindText(name) : SetReplaceText(name)` while the
//  dialog is open+visible, so picking a texture in the texture window fills whichever
//  edit field last had focus (byte_73C380: 1 → Find @0x78, 0 → Replace @0x7C; toggled
//  by the IDB's per-field OnSetFocus handlers sub_415DF0/sub_415E00).
//
//  IDB body (both functions, only the str_set target offset differs):
//     CWnd::UpdateData(g_dlgFind, 1);                 // controls → DDX members
//     if ( g_dlgFind[..].<ctrl member @0x88> )        // dialog populated/active
//     {
//         str_set(&g_dlgFind[..].<CString @0x78|0x7C>, name, name ? strlen(name) : 0);
//         CWnd::UpdateData(g_dlgFind, 0);             // member → control
//     }
//  The kisak dialog stores the two fields directly in the s_frFind / s_frReplace EDIT
//  controls (no separate DDX CString that could diverge), so UpdateData(TRUE) is an
//  implicit no-op and `str_set(member) + UpdateData(FALSE)` collapses to a single
//  ::SetWindowTextA into the control.  The IDB's 0x88 control-member guard maps to "the
//  edit control exists" (HWND non-null) — the same populated-after-OnCreate condition.
void CFindTextureDlg::SetFindText( const char *name )       // 0x415ce0
{
    // UpdateData(TRUE): controls already are the storage (no-op).
    if ( s_frFind )                                          // 0x415cf2 [+0x88] populated guard
    {
        // str_set(&find, name, name ? strlen(name) : 0) + UpdateData(FALSE).
        ::SetWindowTextA( s_frFind, name ? name : "" );
    }
}

void CFindTextureDlg::SetReplaceText( const char *name )    // 0x415d40
{
    if ( s_frReplace )                                       // 0x415d52 [+0x88] populated guard
    {
        ::SetWindowTextA( s_frReplace, name ? name : "" );
    }
}

#ifdef RADIANT_SELFTEST
// ── Deterministic test entry (the findrepl gate) ──────────────────────────────
// Replace material `findName` → `replaceName` across ALL active brush faces (flag&1=0
// → walk active_brushes; flag&8=0 → name-only swap, keep each face's own mapping).
// Returns the number of faces whose current-layer material name == findName BEFORE the
// replace (the expected swap count) via *outBefore, and runs the replace.  A negative
// return means the find name was not present (nothing to do — the gate treats that as a
// map-specific N/A, not a failure).
extern "C" int Radiant_TestFindReplaceCount( const char *name )
{
    // Count faces (over active_brushes, non-prefab, non-patch) whose current-layer
    // material name matches `name`.  (Patches counted separately by the gate if needed.)
    int count = 0;
    int layer = g_qeglobals.current_edit_layer;
    // Read def->faceCount (authoritative) — the instance faceCount cache is 0 on undrawn
    // brushes headless (the instance-vs-def trap; same reason the worker reads def).
    for ( selbrush_t *i = active_brushes.next; i && i != &active_brushes; i = i->next )
    {
        if ( i->owner && i->owner->prefab ) continue;
        if ( !i->def || i->patch ) continue;
        for ( int fi = 0; fi < (int)(unsigned)i->def->faceCount; ++fi )
        {
            MaterialDef *md = &i->def->faces[fi].mtldef[layer];
            if ( ( (md->lyrMtl != nullptr) + (md->radMtl != nullptr) ) != 1 )
                continue;
            const char *nm = (const char *)md->lyrMtl;
            if ( !md->lyrMtl ) nm = md->radMtl ? md->radMtl->name : nullptr;
            if ( nm && _stricmp( nm, name ) == 0 )
                ++count;
        }
    }
    return count;
}

extern "C" int Radiant_TestFindReplaceAll( const char *findName, const char *replaceName )
{
    // flag = 0 → all active brushes, name-match, name-only swap (keep each face's mapping).
    return FindReplaceTextures( findName, replaceName, 0 ) ? 1 : 0;
}
#endif // RADIANT_SELFTEST
