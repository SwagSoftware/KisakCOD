#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Surface inspector for face and patch texture alignment, sample size, and fitting.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ── selection state (select.cpp) ──────────────────────────────────────────────
extern selbrush_t   selected_brushes;                 // engine_stubs (0x23F1864)
extern selface_t   *selFace;                          // select.cpp   (0x73C710)
extern char         g_ptrSelectedFaces_GetSize[4];    // select.cpp   (0x73C714) int alias
#define SEL_FACE_COUNT() (*(int *)g_ptrSelectedFaces_GetSize)

// ── texture edit core (select.cpp / materialdef.cpp / brush.cpp) ───────────────
extern void         Brush_SetTexture( MaterialDef *a1, char a3 );           // 0x48F170
extern void         Brush_SetTextureMapping( texdef_sub_t *a2 );            // 0x48F4F0 (select.cpp) — apply the current-layer texdef to the selection (Undo-bracketed)
extern void         Brush_SetSampleSize( int size );                        // 0x48F800 (select.cpp) — apply sample size to the selection
extern qtexture_s  *MaterialDef_GetLayeredMaterial( MaterialDef *m );       // 0x4314A0
extern LayerMaterialDef *Materialdef_GetName( MaterialDef *m );             // 0x431640
extern void         SetMaterial( const char *name, patchMesh_material *out );// 0x4315C0 (materialdef.cpp)
extern void         TexMatToFakeTexCoords( MaterialDef *def, texdef_sub_t *texDef ); // 0x472DF0 (materialdef.cpp)
extern void         Patch_SetTextureInfo( texdef_sub_t *texDef );           // 0x447760 (pmesh.cpp) — relative patch texdef transform
namespace LayerMat { int GetCurrentLayer( MaterialDef *def ); }             // 0x431B30
extern void         Material_SetMode( int iMode );                          // 0x45B910 (texwnd.cpp)

// g_patch_texdef (IDB 0x23F15F8, patch_texdef_t = 44 B) — the current-texture-window TEMPLATE
// the dialog edits when nothing (or only patches) is selected, PLUS the shared sample-size the
// readout shows / the apply reads back.  {MaterialDef def; int unk3; float sample_size}.
struct patch_texdef_t
{
    MaterialDef def;          // 0x00 (36 B)
    int         unk3;         // 0x24
    float       sample_size;  // 0x28
};
static_assert( sizeof( patch_texdef_t ) == 44, "patch_texdef_t" );
patch_texdef_t g_patch_texdef;      // IDB g_patch_texdef (0x23F15F8)

// Fit the texture to the selected face(s) (Brush_FitTexture 0x4939E0, select.cpp) —
// scale/shift the texdef so the texture spans the face `x`×`y` times.  The "Fit"
// button passes (1,1,0): one tiling across the face, computed bounds, no selection-wide.
extern void         Brush_FitTexture( float x, float y, int a4 );           // 0x4939E0

// ── patch-texturing core (pmesh.cpp / select.cpp) — the Surface Inspector patch row ──
extern void         Select_SetTexture( float *out );                        // 0x456D70 (layer sample size)
extern void         Patch_NaturalizeSelected( bool unk, bool cap, float x, float y ); // 0x447FD0
extern void         Patch_Lightmap_Texturing();                             // 0x448110
extern void         Patch_SetTexturing( float sx, float sy, int mode );     // 0x446B60

extern int          g_nUpdateBits;                    // engine_stubs (0x25D5A74)

// ── multi-layer texmod transaction (the #2 material epic cluster) ──────────────
//  SetTexMods (0x458270) snapshots the current edit-layer's texdef into a scratch slot;
//  SurfaceDlg_Wnd02 (0x4583F0) commits the (possibly edited) scratch back.  The scratch
//  is a standalone MaterialDef-sized region (IDB unk_23F15D4 @ 0x23F15D4) for the
//  current-texture TEMPLATE; per-face the scratch layer is the face's mtldef[3] (the
//  unused 4th MaterialDef slot).  SurfaceInspector_some_boolean_is_up_to_date (0x23F15D2)
//  is the "dialog edits are pending" dirty flag SetTexMods clears and the edit-commit sets.
MaterialDef g_surfTexModScratch;            // IDB unk_23F15D4 (0x23F15D4) — template scratch
char        g_surfTexModDirty = 0;          // IDB SurfaceInspector_some_boolean_is_up_to_date

// PMESH_56/57 thin wrappers (pmesh.cpp) — patch GET/SET, reached via brush_t.patch.
extern void  PMESH_56_extern( patchMesh_t *p );       // 0x44CFB0 (patch save)
extern void  PMESH_57_extern( patchMesh_t *p );       // 0x44D0C0 (patch restore + Patch_Rebuild)

#define SURFACEDLG_CPP "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp"

// The radiant verbose Assert stub (engine_stubs.cpp): type 0 = log-to-stderr + CONTINUE.
extern void Assert( const char *file, int line, int type, const char *fmt, ... );
extern int  Sys_Printf( const char *fmt, ... );       // win_qe3.cpp (status/console print)

// ── surface-inspector globals (IDB g_surfwin 0x23F1624 / the CSurfaceDlg instance) ──
//  g_surfwin = the inspector HWND when open, 0 when closed (map.cpp reads it to know
//  whether to refresh the inspector after a load).  g_bNewFace (0x23F15D0) gates the
//  "edit the picked face" vs "edit the current-texture window" path (the binary derives
//  it from prefs m_bFace; this build keeps it on so a selected face is what you edit).
int  g_surfwin   = 0;
char g_bNewFace  = 1;

CSurfaceDlg *g_pSurfDlg = nullptr;     // the hand-built inspector instance (NULL until opened)

// ══════════════════════════════════════════════════════════════════════════════
//  texdef CORE — read / apply a face's texture alignment (headless-safe).
//
//  texdef_sub_t (qedefs.h, 28 bytes / 7 floats): size[2] scale-as-texels, shift[2]
//  texel shift, rotate (deg), unk3, sample_size.  The .map face line writes the
//  current layer's block as `<material> size0 size1 shift0 shift1 rotate unk3`
//  (Brush_Write 0x474e90); the camera reads size/shift/rotate through Face_MoveTexture.
// ══════════════════════════════════════════════════════════════════════════════

// SurfaceInspector_GetMaterialDef (0x457950) essence — WHICH MaterialDef the inspector
// edits for the current selection (no dialog-field reads; those are committed separately):
//   * a face is selected (g_bNewFace) → that face's mtldef[current_edit_layer]
//     (a pointer INTO the face def, so edits land on the face; Brush_SetTexture then
//      propagates it to every selected face),
//   * else → the editor's current-texture-window template (random_texture_stuff[layer]).
//  Returns NULL only when nothing is selected at all (the apply path then no-ops).
MaterialDef *Surf_TargetMaterialDef()
{
    if ( g_bNewFace && SEL_FACE_COUNT() > 0 )
    {
        selbrush_t *b = selFace[0].brush;
        if ( b && b->def )
        {
            // RESTORED (SurfaceDlg.cpp:420/421, type-0 = log+continue): the binary's
            // SurfaceInspector_GetMaterialDef asserts the picked face is still live + the
            // instance/def versions are in sync before reading the DEF face's mtldef.  Verbose:
            // the binary string uses '.' member syntax ("selFace.face …") that a stringized '->'
            // expr can't byte-match (same case as texturebar.cpp:160/161).
            if ( selFace[0].face != &b->faces[selFace[0].index] )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp",
                        420, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
            if ( b->version != b->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp",
                        421, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
            return &b->def->faces[selFace[0].index].mtldef[g_qeglobals.current_edit_layer];
        }
    }
    if ( selected_brushes.next != &selected_brushes )
        return &g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].mtl;
    // Nothing selected — fall back to the current-texture template so the inspector still
    // shows something coherent; the apply path checks SEL state before mutating anything.
    return &g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].mtl;
}
// SurfaceDlg_GetMaterialSize (0x456f50) helper — the editor texture's pixel dimensions
// (autoTexScale), falling back to 512×512 when the material has no resolved image.
void Surf_MaterialWH( MaterialDef *md, int *w, int *h )
{
    qtexture_s *lm = md ? MaterialDef_GetLayeredMaterial( md ) : nullptr;
    if ( lm ) { *w = lm->width;  *h = lm->height; }
    else      { *w = 512;        *h = 512; }
}

// ── Surface-inspector sample-size readouts ────────────────────────────────────
//   Ported verbatim from IDA; feed the inspector's sample-size field + patch name.
//   The IDB's SurfaceDlg_GetMaterialSize used the OLD flattened face_t (28-byte texdef
//   slots); the corrected port face_t has 36-byte mtldef[] slots, so the per-layer step
//   is `&face->mtldef[layer]` (matching Surf_ReadTexdef's proven idiom).
extern int   QE_SingleBrush();                                  // qe3.cpp 0x48C8B0
extern void  SetMaterial( const char *name, patchMesh_material *out ); // 0x4315C0
extern LayerMaterialDef *Patch_GetTextureName();                // pmesh.cpp 0x448670 (below)

// SurfaceDlg_GetMaterialSize (0x456F50) — the face's proportional texel scale
// (size[0]/width) when the scale is isotropic (size[1]*width == height*size[0]), else 0.
float SurfaceDlg_GetMaterialSize( face_t *f )
{
    if ( !f ) Assert( SURFACEDLG_CPP, 166, 0, "%s", "f" );
    MaterialDef *md = &f->mtldef[ LayerMat::GetCurrentLayer( &f->mtldef[0] ) ];
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( &f->mtldef[0] );
    int width  = lm ? lm->width  : 512;
    int height = lm ? lm->height : 512;
    float *size = md->mat_texDef.size;
    if ( size[1] * (float)width != (double)height * size[0] )
        return 0.0f;
    if ( !width ) Assert( SURFACEDLG_CPP, 173, 0, "%s", "size[0]" );
    return (float)( size[0] / (float)width );
}

// SurfaceDlg_GetMaterialSize_Patch (0x457010) — patch or brush texel scale.  For a patch
// symbiont, reads the cached sample (bDirty gates it); for a brush, the common scale of
// all its faces (0 if they disagree).
float SurfaceDlg_GetMaterialSize_Patch( brush_t *b )
{
    if ( !b ) Assert( SURFACEDLG_CPP, 183, 0, "%s", "b" );
    patchMesh_t *patch = b->patch;
    if ( patch )
    {
        if ( !patch->bDirty )
            return 0.0f;
        return *(float *)&patch->size_of_struct_0x504C;
    }
    float sz = SurfaceDlg_GetMaterialSize( &b->faces[0] );
    if ( sz == 0.0f )
        return 0.0f;
    for ( unsigned int i = 1; i < (unsigned int)b->faceCount; ++i )
        if ( SurfaceDlg_GetMaterialSize( &b->faces[i] ) != sz )
            return 0.0f;
    return sz;
}

// GetBrushSampleSize (0x4570C0) — the common patch/brush sample size across the whole
// selection (0 unless every selected brush shares it).
float GetBrushSampleSize()
{
    if ( selected_brushes.next == &selected_brushes )
        return 0.0f;
    float sz = SurfaceDlg_GetMaterialSize_Patch( selected_brushes.next->def );
    if ( sz == 0.0f )
        return 0.0f;
    for ( selbrush_t *n = selected_brushes.next->next; n != &selected_brushes; n = n->next )
        if ( SurfaceDlg_GetMaterialSize_Patch( n->def ) != sz )
            return 0.0f;
    return sz;
}

// GetBrushfaceSampleSize (0x457170) — the common sample size across all SELECTED FACES
// (patch faces via _Patch, brush faces via GetMaterialSize); 0 unless all agree.
float GetBrushfaceSampleSize()
{
    int count = SEL_FACE_COUNT();
    if ( count <= 0 )
        return 0.0f;
    selbrush_t *b0 = selFace[0].brush;
    float sz = b0->patch ? SurfaceDlg_GetMaterialSize_Patch( b0->def )
                         : SurfaceDlg_GetMaterialSize( &b0->def->faces[selFace[0].index] );
    if ( sz == 0.0f )
        return 0.0f;
    for ( int i = 1; i < count; ++i )
    {
        selbrush_t *b = selFace[i].brush;
        float s = b->patch ? SurfaceDlg_GetMaterialSize_Patch( b->def )
                           : SurfaceDlg_GetMaterialSize( &b->def->faces[selFace[i].index] );
        if ( s != sz )
            return 0.0f;
    }
    return sz;
}

// Read the current selection's texdef (the picked face / current-texture template).
// Returns false when there is no editable MaterialDef.  `out` floats: size0 size1
// shift0 shift1 rotate (the .map order minus unk3/sample).
bool Surf_ReadTexdef( float out[5], int *wOut, int *hOut )
{
    MaterialDef *md = Surf_TargetMaterialDef();
    // "No editable MaterialDef" includes an UNINITIALIZED current-texture template: when the
    // inspector is opened with nothing selected and no current texture yet, Surf_TargetMaterialDef
    // returns &random_texture_stuff[layer].mtl with BOTH lyrMtl and radMtl null. MtlDef_IsValid
    // (materialdef.cpp) requires EXACTLY ONE set, so LayerMat::GetCurrentLayer would trip its
    // level-0 iassert (debug-break). Treat an invalid def as "not editable" → empty fields, no
    // assert (the binary's GetCurrentLayer logs-and-returns-0 here; this is the cleaner equivalent).
    if ( !md || ( ( md->lyrMtl != nullptr ) + ( md->radMtl != nullptr ) ) != 1 )
        return false;
    int layer = LayerMat::GetCurrentLayer( md );
    texdef_sub_t *td = &md->mat_texDef + layer;
    out[0] = td->size[0];  out[1] = td->size[1];
    out[2] = td->shift[0]; out[3] = td->shift[1];
    out[4] = td->rotate;
    if ( wOut || hOut ) { int w, h; Surf_MaterialWH( md, &w, &h ); if ( wOut ) *wOut = w; if ( hOut ) *hOut = h; }
    return true;
}

// Apply STORED texdef values (size in texels, shift in texels, rotate in degrees) to the
// selected face(s).  This is the heart of the inspector: write the target MaterialDef's
// texdef, then Brush_SetTexture (copies it onto every selected face + rebuilds windings /
// faceVis / bumps version), then invalidate every view so the camera re-projects the
// texture through Face_MoveTexture (Stage-1d faithful texcoords).  Headless-safe.
void Surf_ApplyTexdefRaw( float size0, float size1, float shift0, float shift1, float rotate )
{
    MaterialDef *md = Surf_TargetMaterialDef();
    if ( !md )
        return;
    int layer = LayerMat::GetCurrentLayer( md );
    texdef_sub_t *td = &md->mat_texDef + layer;
    td->size[0]  = ( size0 != 0.0f ) ? size0 : 0.25f;   // 0 size ⇒ Face_MoveTexture forces 128; keep sane
    td->size[1]  = ( size1 != 0.0f ) ? size1 : 0.25f;
    td->shift[0] = shift0;
    td->shift[1] = shift1;
    td->rotate   = rotate;

    Brush_SetTexture( md, 0 );   // propagate to selected faces + rebuild geometry/version
    g_nUpdateBits = -1;          // invalidate every view → camera re-projects (Face_MoveTexture)
}

// ══════════════════════════════════════════════════════════════════════════════
//  MULTI-LAYER TEXMOD TRANSACTION (the #2 material epic) — SetTexMods / SurfaceDlg_Wnd02
//  + the per-brush PostDoSurface02 / PostDoSurface02_Patch helpers.  Faithful ports of
//  the IDB cluster (port 13343); every per-layer stride below was traced from the DISASM
//  (hex-rays renders them as `planepts[3*layer+3]` / `pad_0x0088[8]` artifacts):
//
//   * the per-FACE scratch is face->mtldef[3] (the unused 4th MaterialDef slot):
//       face + 0x90 = face + 0x24 + 3*0x24 = mtldef[3]  (disasm `add edi,90h` + rep movsd 0x24).
//   * the per-FACE source/dest layer is face->mtldef[current_edit_layer]:
//       face + (9*layer+9)*4 = face + 0x24 + layer*0x24 = mtldef[layer]
//       (disasm `lea eax,[eax+eax*8+9]; lea ...,[face+eax*4]`).
//   * the TEMPLATE scratch is g_surfTexModScratch (IDB unk_23F15D4); the template layer
//       block is random_texture_stuff[layer] (stride 0x834 = 2100 B; disasm `imul esi,834h`).
//   * MaterialDef = 0x24 = 36 bytes (the rep-movsd count is always 9 dwords).
// ══════════════════════════════════════════════════════════════════════════════

// SurfaceInspector::PostDoSurface02 (0x47CA50) — SAVE one brush def's faces: copy every
// face's CURRENT-layer mtldef into its mtldef[3] scratch, then PMESH_56 on the patch.
//   qmemcpy(&face->mtldef[3], &face->mtldef[current], 0x24)  → mtldef[3] ← mtldef[current]
static void Surf_PostDoSurface02( brush_t *def )
{
    if ( !def )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp", 5481, 0, "%s", "b" );
    for ( unsigned int i = 0; i < (unsigned int)def->faceCount; ++i )
    {
        face_t *f = &def->faces[i];
        if ( !f )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp", 5464, 0, "%s", "f" );
        f->mtldef[3] = f->mtldef[g_qeglobals.current_edit_layer];   // mtldef[3] ← mtldef[current]
    }
    if ( def->patch )
        PMESH_56_extern( def->patch );
}

// SurfaceDlg_PostDoSurface02_Patch (0x47CAF0) — RESTORE one brush def's faces: copy each
// face's mtldef[3] scratch BACK into its current-layer mtldef, ++def->version, then PMESH_57.
//   qmemcpy(&face->mtldef[current], &face->mtldef[3], 0x24)  → mtldef[current] ← mtldef[3]
static void Surf_PostDoSurface02_Patch( brush_t *def )
{
    if ( !def )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp", 5495, 0, "%s", "b" );
    for ( unsigned int i = 0; i < (unsigned int)def->faceCount; ++i )
    {
        face_t *f = &def->faces[i];
        if ( !f )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp", 5471, 0, "%s", "f" );
        f->mtldef[g_qeglobals.current_edit_layer] = f->mtldef[3];   // mtldef[current] ← mtldef[3]
        ++def->version;                                             // ++*(WORD*)(def+0x4E)
    }
    if ( def->patch )
        PMESH_57_extern( def->patch );
}

// SurfaceInspector::SetTexMods (0x458270) — open / layer-switch SAVE pass.  Snapshot the
// current-texture TEMPLATE into the scratch, then save every selected brush's faces (mtldef
// [current]→mtldef[3]) and every selected FACE's current-layer texdef into its mtldef[3]
// scratch.  Clears the dirty flag.  (qmemcpy direction: scratch ← live, verified from disasm.)
void SurfaceInspector_SetTexMods()
{
    if ( !g_surfwin )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp", 683, 0, "%s", "surfDlgGlob.hwnd" );

    // scratch ← random_texture_stuff[current_edit_layer].mtl  (rep movsd, dst=scratch)
    g_surfTexModScratch = g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].mtl;

    g_surfTexModDirty = 0;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        Surf_PostDoSurface02( b->def );

    int count = SEL_FACE_COUNT();
    for ( int i = 0; i < count; ++i )
    {
        selbrush_t *brush = selFace[i].brush;
        int index = selFace[i].index;
        if ( selFace[i].face != &brush->faces[index] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp",
                    695, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( brush->version != brush->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp",
                    696, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
        face_t *f = &brush->def->faces[index];
        if ( !f )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp", 5464, 0, "%s", "f" );
        f->mtldef[3] = f->mtldef[g_qeglobals.current_edit_layer];   // mtldef[3] ← mtldef[current]
    }
}

// SurfaceDlg_Wnd02 (0x4583F0) — close / commit APPLY pass.  Restore the current-texture
// TEMPLATE from the scratch (always), then — only if the dirty flag is set — commit every
// selected brush's faces (mtldef[3]→mtldef[current], ++version, PMESH_57) and every selected
// FACE's mtldef[3] back into its current-layer texdef (++version).  Mirror of SetTexMods.
void SurfaceInspector_Wnd02()
{
    if ( !g_surfwin )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp", 708, 0, "%s", "surfDlgGlob.hwnd" );

    bool wasDirty = ( g_surfTexModDirty != 0 );
    // random_texture_stuff[current_edit_layer].mtl ← scratch  (rep movsd, src=scratch)
    g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].mtl = g_surfTexModScratch;
    if ( !wasDirty )
        return;

    g_surfTexModDirty = 0;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        Surf_PostDoSurface02_Patch( b->def );

    int count = SEL_FACE_COUNT();
    for ( int i = 0; i < count; ++i )
    {
        selbrush_t *brush = selFace[i].brush;
        int index = selFace[i].index;
        if ( selFace[i].face != &brush->faces[index] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp",
                    722, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( brush->version != brush->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\SurfaceDlg.cpp",
                    723, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
        brush_t *def = brush->def;
        face_t *f = &def->faces[index];
        if ( !f )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp", 5471, 0, "%s", "f" );
        f->mtldef[g_qeglobals.current_edit_layer] = f->mtldef[3];   // mtldef[current] ← mtldef[3]
        ++def->version;
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  CSurfaceDlg — the Surface Inspector, a modeless CDialog on IDD_SURFACE_INSPECTOR (116).
//  1:1 with the binary (CDialog::Create(IDD_SURFACE_INSP) in DoSurface 0x4585d0); the RC
//  template + control ids are the binary's (res/radiant.rc).  The data flow mirrors the
//  CSurfaceDlg cluster exactly:
//    * SurfaceInspector_GetMaterialDef (0x457950) — READ every edit control into the target
//      texdef (stretch*width/height → texels for non-patch), update the "Repeats in" readouts,
//      SetMaterial + TexMatToFakeTexCoords, set the dirty flag; returns the MaterialDef.
//    * GetTexMods (0x458960) — on Done/Apply + on each spin: if the "texdef edited" flag is up,
//      GetMaterialDef then Brush_SetTextureMapping (propagate to the selection); if the
//      "sample edited" flag is up, Brush_SetSampleSize; then Select_SetTexture_2 refresh.
//    * Select_SetTexture_2 (0x4572d0) — WRITE the target texdef back into the controls.
// ══════════════════════════════════════════════════════════════════════════════

// %.6g float → control (SprintFormat_SetText 0x457270 essence).
static void SI_SetDlgFloat( CWnd *dlg, int id, float v )
{
    char buf[64];
    sprintf( buf, "%.6g", v );
    dlg->SetDlgItemText( id, buf );
}
static float SI_GetDlgFloat( CWnd *dlg, int id )
{
    char buf[128] = { 0 };
    ::GetDlgItemTextA( dlg->GetSafeHwnd(), id, buf, sizeof( buf ) - 1 );
    return (float)atof( buf );
}

// SurfaceInspector_GetMaterialDef (0x457950) — READ the dialog's edit controls into the
// current-layer texdef of the target MaterialDef (the picked face's mtldef[layer], or the
// current-texture template / g_patch_texdef for a patch/no-selection), then TexMatToFakeTexCoords.
// Returns the MaterialDef (NULL only when nothing editable).  For a NON-patch selection the
// "stretch" fields are texel-scale = size/width, so they are multiplied by width/height back to
// texels; for a patch they are stored raw.  Sample size always lands in g_patch_texdef.sample_size.
static MaterialDef *SI_ReadControlsIntoTexdef( CSurfaceDlg *dlg )
{
    // WHICH MaterialDef + patch-mode (Surf_TargetMaterialDef == SurfaceInspector_GetMaterialDef's
    // target resolution; also sets m_bPatchMode for the patch/no-face-selection path).
    bool onlyPatches = ( SEL_FACE_COUNT() <= 0 && selected_brushes.next != &selected_brushes );
    if ( onlyPatches )
        for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
            if ( !b->patch ) { onlyPatches = false; break; }

    MaterialDef *md;
    if ( onlyPatches )
    {
        // patch-only selection → the shared g_patch_texdef template (binary path).
        md = &g_patch_texdef.def;
        dlg->m_bPatchMode = 1;
    }
    else
    {
        md = Surf_TargetMaterialDef();
        dlg->m_bPatchMode = 0;
    }
    if ( !md )
        return nullptr;

    int width = 512, height = 512;
    Surf_MaterialWH( md, &width, &height );
    if ( width  <= 0 ) width  = 512;
    if ( height <= 0 ) height = 512;

    // Name → SetMaterial (empty/blank → "$default", then reflect the resolved name back).
    char name[132] = { 0 };
    ::GetDlgItemTextA( dlg->GetSafeHwnd(), IDC_SURFACE_INSP_CURR_TEX, name, 127 );
    if ( (unsigned char)name[0] > 32 )
    {
        SetMaterial( name, (patchMesh_material *)md );
    }
    else
    {
        SetMaterial( "$default", (patchMesh_material *)md );
        LayerMaterialDef *n = Materialdef_GetName( md );
        dlg->SetDlgItemText( IDC_SURFACE_INSP_CURR_TEX, n ? (const char *)n : "" );
    }

    texdef_sub_t *td = &md->mat_texDef + LayerMat::GetCurrentLayer( md );
    td->shift[0] = SI_GetDlgFloat( dlg, IDC_SURFACE_INSP_HORZ_SHIFT_IN );
    td->shift[1] = SI_GetDlgFloat( dlg, IDC_SURFACE_INSP_VERT_SHIFT_IN );

    float sx = SI_GetDlgFloat( dlg, IDC_SURFACE_INSP_HORZ_STRETCH );
    td->size[0] = dlg->m_bPatchMode ? sx : sx * (float)width;
    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_TXT_REPEATS_X, td->size[0] );   // "Repeats in" X readout

    float sy = SI_GetDlgFloat( dlg, IDC_SURFACE_INSP_VERT_STRETCH );
    td->size[1] = dlg->m_bPatchMode ? sy : sy * (float)height;
    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_TXT_REPEATS_Y, td->size[1] );   // "Repeats in" Y readout

    td->rotate = SI_GetDlgFloat( dlg, IDC_SURFACE_INSP_ROTATE );
    g_patch_texdef.sample_size = SI_GetDlgFloat( dlg, IDC_SURFACE_INSP_SAMPLE_SIZE );

    TexMatToFakeTexCoords( md, td );
    g_surfTexModDirty = 1;
    return md;
}

// Select_SetTexture_2 (0x4572d0) — WRITE the target texdef back into the dialog controls.
// Public (texwnd.cpp / map.cpp / Material_SetMode call it as Surf_RefreshFields).  Shift is
// shown in texels; "stretch" = size/width (size/height); rotate in degrees; sample-size the
// shared g_patch_texdef.sample_size ("" when 0).  Also fills the "Repeats in" readouts (raw size).
void Surf_RefreshFields()
{
    if ( !g_surfwin || !g_pSurfDlg || !::IsWindow( g_pSurfDlg->GetSafeHwnd() ) )
        return;
    CSurfaceDlg *dlg = g_pSurfDlg;

    // Target + patch-mode (mirrors Select_SetTexture_2's target resolution).
    bool onlyPatches = ( SEL_FACE_COUNT() <= 0 && selected_brushes.next != &selected_brushes );
    if ( onlyPatches )
        for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
            if ( !b->patch ) { onlyPatches = false; break; }

    MaterialDef *md;
    if ( onlyPatches )
    {
        // patch-only selection → the shared g_patch_texdef template.  Populate its material:
        // single selected patch → Patch_GetTextureName; else copy the current-texture template.
        MaterialDef *tmpl = &g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].mtl;
        if ( QE_SingleBrush() )
            SetMaterial( (const char *)Patch_GetTextureName(), (patchMesh_material *)&g_patch_texdef );
        else
        {
            g_patch_texdef.def.lyrMtl = tmpl->lyrMtl;
            g_patch_texdef.def.radMtl = tmpl->radMtl;
        }
        md = &g_patch_texdef.def;
        dlg->m_bPatchMode = 1;
        g_patch_texdef.sample_size = GetBrushSampleSize();
    }
    else
    {
        md = Surf_TargetMaterialDef();
        dlg->m_bPatchMode = 0;
        g_patch_texdef.sample_size = ( g_bNewFace && SEL_FACE_COUNT() > 0 )
                                   ? GetBrushfaceSampleSize()
                                   : GetBrushSampleSize();
    }
    if ( !md || ( ( md->lyrMtl != nullptr ) + ( md->radMtl != nullptr ) ) != 1 )
        return;   // no editable MaterialDef (uninitialized template) — leave fields as-is

    int width = 512, height = 512;
    Surf_MaterialWH( md, &width, &height );
    if ( width  <= 0 ) width  = 512;
    if ( height <= 0 ) height = 512;

    ::SendMessageA( dlg->GetSafeHwnd(), WM_SETREDRAW, 0, 0 );

    texdef_sub_t *td = &md->mat_texDef + LayerMat::GetCurrentLayer( md );
    if ( !dlg->m_bPatchMode )
        TexMatToFakeTexCoords( md, td );

    // Name: single selected patch → Patch_GetTextureName; else the MaterialDef name.
    const char *name;
    if ( dlg->m_bPatchMode && QE_SingleBrush() )
        name = (const char *)Patch_GetTextureName();
    else
        name = (const char *)Materialdef_GetName( md );
    dlg->SetDlgItemText( IDC_SURFACE_INSP_CURR_TEX, name ? name : "" );

    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_HORZ_SHIFT_IN, td->shift[0] );
    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_VERT_SHIFT_IN, td->shift[1] );

    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_HORZ_STRETCH,
                    dlg->m_bPatchMode ? td->size[0] : td->size[0] / (float)width );
    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_TXT_REPEATS_X, td->size[0] );
    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_VERT_STRETCH,
                    dlg->m_bPatchMode ? td->size[1] : td->size[1] / (float)height );
    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_TXT_REPEATS_Y, td->size[1] );
    SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_ROTATE, td->rotate );

    if ( g_patch_texdef.sample_size == 0.0f )
        dlg->SetDlgItemText( IDC_SURFACE_INSP_SAMPLE_SIZE, "" );
    else
        SI_SetDlgFloat( dlg, IDC_SURFACE_INSP_SAMPLE_SIZE, g_patch_texdef.sample_size );

    ::SendMessageA( dlg->GetSafeHwnd(), WM_SETREDRAW, 1, 0 );
    ::InvalidateRect( dlg->GetSafeHwnd(), nullptr, TRUE );
}

// SurfaceInspector::UpdateSurfaceDialog (0x458590) — the global "selection/texture changed"
// refresh, called from Brush_AddToList2 / Brush_RemoveFromList / drag apply etc.
// Binary body: if (g_surfwin) { SurfaceInspector::SetTexMods(); Select_SetTexture_2(&g_dlgSurface); }
// then if (g_pParentWnd && m_wndTextureBar.m_hWnd) CTextureBar::GetSurfaceAttributes(&bar).
// Port names: SetTexMods → SurfaceInspector_SetTexMods; Select_SetTexture_2 → Surf_RefreshFields.
extern CMainFrame *g_pParentWnd;                                 // 0x25D5A70
namespace SurfaceInspector
{
    void UpdateSurfaceDialog()
    {
        if ( g_surfwin )
        {
            SurfaceInspector_SetTexMods();          // 0x45859a  SurfaceInspector::SetTexMods
            Surf_RefreshFields();                   // 0x4585a4  Select_SetTexture_2
        }
        if ( g_pParentWnd && g_pParentWnd->m_wndTextureBar.m_hWnd )     // 0x4585b7
            CTextureBar::GetSurfaceAttributes( &g_pParentWnd->m_wndTextureBar );   // 0x4585bd
    }
}

// GetTexMods (0x458960) — commit any pending dialog edits to the selection.  Called on
// Done/Apply and after each spin.  m_texdefDirty (edit changed) → read controls into the texdef
// (GetMaterialDef) then Brush_SetTextureMapping (Undo-bracketed propagate to the selection);
// m_sampleDirty (sample-size changed) → Brush_SetSampleSize; then refresh.
static void SI_GetTexMods( CSurfaceDlg *dlg )
{
    if ( !dlg->m_texdefDirty && !dlg->m_sampleDirty )
        return;
    MaterialDef *md = SI_ReadControlsIntoTexdef( dlg );
    int layer = md ? LayerMat::GetCurrentLayer( md ) : 0;
    if ( dlg->m_texdefDirty )
    {
        if ( md )
            Brush_SetTextureMapping( &md->mat_texDef + layer );
        dlg->m_texdefDirty = 0;
    }
    if ( dlg->m_sampleDirty )
    {
        if ( g_patch_texdef.sample_size != 0.0f )
        {
            Brush_SetSampleSize( (int)g_patch_texdef.sample_size );
            dlg->m_sampleDirty = 0;
            Surf_RefreshFields();
            return;
        }
        dlg->m_sampleDirty = 0;
    }
    Surf_RefreshFields();
}

// UpdateSpinners (0x457cf0) — a spin arrow stepped the texdef.  The non-patch path (the common
// case) steps the current-layer texdef of SurfaceInspector_GetMaterialDef's target: stretch
// size[0]/size[1] ±8, shift[0]/shift[1] ±1, rotate ±45 (0..360 wrap), sample-size ±4 (min 4).
// The patch path builds a relative texdef_sub_t and Patch_SetTextureInfo's it.  Then refresh +
// Brush_SetTexture + clear the edit-dirty flag (binary xx9_1=0).
static void SI_UpdateSpinners( CSurfaceDlg *dlg, int idFrom, bool up )
{
    if ( dlg->m_bPatchMode )
    {
        // patch path (Patch_SetTextureInfo relative transform).
        SI_ReadControlsIntoTexdef( dlg );   // UpdateGPatchTexdef equivalent (commit current fields)
        texdef_sub_t *pt = &g_patch_texdef.def.mat_texDef + LayerMat::GetCurrentLayer( &g_patch_texdef.def );
        texdef_sub_t rel; memset( &rel, 0, sizeof( rel ) );
        switch ( idFrom )
        {
            case IDC_SURFACE_INSP_ROTATE_SPIN:        rel.rotate  = up ?  pt->rotate  : -pt->rotate;  break;
            case IDC_SURFACE_INSP_HORZ_STRETCH_SPIN:  rel.size[0] = up ? 1.0f - pt->size[0] : pt->size[0] + 1.0f; break;
            case IDC_SURFACE_INSP_VERT_STRETCH_SPIN:  rel.size[1] = up ? 1.0f - pt->size[1] : pt->size[1] + 1.0f; break;
            case IDC_SURFACE_INSP_HORZ_SHIFT_SPIN:    rel.shift[0] = up ?  pt->shift[0] : -pt->shift[0]; break;
            case IDC_SURFACE_INSP_VERT_SHIFT_SPIN:    rel.shift[1] = up ?  pt->shift[1] : -pt->shift[1]; break;
            case IDC_SURFACE_INSP_SAMPLE_SIZE_SPIN:
            {
                if ( g_patch_texdef.sample_size == 0.0f )
                    break;
                float s = g_patch_texdef.sample_size + ( up ? 4.0f : -4.0f );
                if ( s < 4.0f ) s = 4.0f;
                g_patch_texdef.sample_size = s;
                Brush_SetSampleSize( (int)g_patch_texdef.sample_size );
                Surf_RefreshFields();
                dlg->m_sampleDirty = 0;
                return;
            }
        }
        MaterialDef *md = &g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].mtl;
        Patch_SetTextureInfo( &rel );
        Surf_RefreshFields();
        g_surfTexModDirty = 1;
        Brush_SetTexture( md, 0 );
        dlg->m_texdefDirty = 0;
        return;
    }

    // brush / face path — the binary calls SurfaceInspector_GetMaterialDef (reads the current
    // control values into the target texdef) FIRST, then steps that texdef by the delta.
    MaterialDef *md = SI_ReadControlsIntoTexdef( dlg );
    if ( !md )
        return;
    texdef_sub_t *td = &md->mat_texDef + LayerMat::GetCurrentLayer( md );
    switch ( idFrom )
    {
        case IDC_SURFACE_INSP_ROTATE_SPIN:
            td->rotate += up ? 45.0f : -45.0f;
            if ( td->rotate <    0.0f ) td->rotate += 360.0f;
            if ( td->rotate >= 360.0f ) td->rotate -= 360.0f;
            break;
        case IDC_SURFACE_INSP_HORZ_STRETCH_SPIN: td->size[0]  += up ?  8.0f : -8.0f; break;
        case IDC_SURFACE_INSP_VERT_STRETCH_SPIN: td->size[1]  += up ?  8.0f : -8.0f; break;
        case IDC_SURFACE_INSP_HORZ_SHIFT_SPIN:   td->shift[0] += up ?  1.0f : -1.0f; break;
        case IDC_SURFACE_INSP_VERT_SHIFT_SPIN:   td->shift[1] += up ?  1.0f : -1.0f; break;
        case IDC_SURFACE_INSP_SAMPLE_SIZE_SPIN:
        {
            if ( g_patch_texdef.sample_size == 0.0f )
                break;
            float s = g_patch_texdef.sample_size + ( up ? 4.0f : -4.0f );
            if ( s < 4.0f ) s = 4.0f;
            g_patch_texdef.sample_size = s;
            Brush_SetSampleSize( (int)g_patch_texdef.sample_size );
            Surf_RefreshFields();
            dlg->m_sampleDirty = 0;
            g_surfTexModDirty = 1;
            return;
        }
        default:
            break;
    }
    Surf_RefreshFields();
    g_surfTexModDirty = 1;
    Brush_SetTexture( md, 0 );
    dlg->m_texdefDirty = 0;
}

BEGIN_MESSAGE_MAP( CSurfaceDlg, CDialog )
    // WM_COMMAND / BN_CLICKED (binary msgmap 0x6e2e58):
    ON_BN_CLICKED( IDC_SURFACE_INSP_BTN_DONE,    &CSurfaceDlg::OnDone )       // 1489 → OnOK   (0x4589e0)
    ON_BN_CLICKED( IDC_SURFACE_INSP_BTN_CANCEL,  &CSurfaceDlg::OnCancel2 )    // 1198 → sub_458AB0
    ON_BN_CLICKED( IDC_SURFACE_INSP_BTN_CAP,     &CSurfaceDlg::OnCap )        // 1282 → OnCap  (0x458b40)
    ON_BN_CLICKED( IDC_SURFACE_INSP_BTN_NATURAL, &CSurfaceDlg::OnNaturalize ) // 1284 → OnNaturalize (0x458c60)
    ON_BN_CLICKED( IDC_SURFACE_INSP_BTN_SET,     &CSurfaceDlg::OnSet )        // 1283 → OnSet  (0x458cb0)
    ON_BN_CLICKED( IDC_SURFACE_INSP_BTN_FIT,     &CSurfaceDlg::OnFit )        // 1286 → OnFit  (0x458de0)
    ON_BN_CLICKED( IDC_SURFACE_INSP_BTN_LMAP,    &CSurfaceDlg::OnLightmap )   // 1287 → OnLightmap (0x458ca0)
    // WM_COMMAND / EN_CHANGE (0x458e10 / 0x458e20 — set the "edit changed" dirty flags):
    ON_EN_CHANGE( IDC_SURFACE_INSP_HORZ_STRETCH,  &CSurfaceDlg::OnTexdefEdited )  // 1211
    ON_EN_CHANGE( IDC_SURFACE_INSP_VERT_STRETCH,  &CSurfaceDlg::OnTexdefEdited )  // 1213
    ON_EN_CHANGE( IDC_SURFACE_INSP_HORZ_SHIFT_IN, &CSurfaceDlg::OnTexdefEdited )  // 1195
    ON_EN_CHANGE( IDC_SURFACE_INSP_VERT_SHIFT_IN, &CSurfaceDlg::OnTexdefEdited )  // 1196
    ON_EN_CHANGE( IDC_SURFACE_INSP_ROTATE,        &CSurfaceDlg::OnTexdefEdited )  // 1217
    ON_EN_CHANGE( IDC_SURFACE_INSP_SAMPLE_SIZE,   &CSurfaceDlg::OnSampleEdited )  // 1035
    // WM_NOTIFY / UDN_DELTAPOS spin arrows (0x458b10 → UpdateSpinners 0x457cf0):
    ON_NOTIFY( UDN_DELTAPOS, IDC_SURFACE_INSP_HORZ_SHIFT_SPIN,   &CSurfaceDlg::OnDeltaPosSpin )  // 1248
    ON_NOTIFY( UDN_DELTAPOS, IDC_SURFACE_INSP_VERT_SHIFT_SPIN,   &CSurfaceDlg::OnDeltaPosSpin )  // 1251
    ON_NOTIFY( UDN_DELTAPOS, IDC_SURFACE_INSP_HORZ_STRETCH_SPIN, &CSurfaceDlg::OnDeltaPosSpin )  // 1257
    ON_NOTIFY( UDN_DELTAPOS, IDC_SURFACE_INSP_VERT_STRETCH_SPIN, &CSurfaceDlg::OnDeltaPosSpin )  // 1254
    ON_NOTIFY( UDN_DELTAPOS, IDC_SURFACE_INSP_ROTATE_SPIN,       &CSurfaceDlg::OnDeltaPosSpin )  // 1259
    ON_NOTIFY( UDN_DELTAPOS, IDC_SURFACE_INSP_SAMPLE_SIZE_SPIN,  &CSurfaceDlg::OnDeltaPosSpin )  // 1260
    ON_WM_DESTROY()
    ON_WM_CLOSE()               // [X] title button → OnClose → OnCancel (real destroy)
END_MESSAGE_MAP()

CSurfaceDlg::CSurfaceDlg()
    : CDialog( IDD_SURFACE_INSPECTOR, nullptr ),
      m_bPatchMode( 0 ), m_texdefDirty( 0 ), m_sampleDirty( 0 )
{
}

// DoDataExchange (0x456c00) — DDX_Control on the six spin controls (so their buddy
// arrows drive the edits) + DDX_Text on the tex-repeat x/y edits.  The spins are simple
// child controls here; the up-down notifications are routed by the message map, so the
// DDX_Control bindings only need to exist to satisfy MFC's subclassing of the up-downs.
void CSurfaceDlg::DoDataExchange( CDataExchange *pDX )
{
    CDialog::DoDataExchange( pDX );
    // (Binary binds the spins to CSpinButtonCtrl members via DDX_Control; the deltas are
    //  handled through the message map, so no member spinners are needed here.)
}

// OnInitDialog (0x458710) — set the up-down ranges, seed the tex-repeat edits + terrain-mode
// radio from the cached g_qeglobals values, then refresh the fields from the selection.
BOOL CSurfaceDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    g_surfwin = (int)(intptr_t)GetSafeHwnd();
    Surf_RefreshFields();

    // Up-down ranges (binary sets 0..1000 for the texdef spins, 0..1024 for tex-repeat).
    static const int spins[] = {
        IDC_SURFACE_INSP_VERT_SHIFT_SPIN, IDC_SURFACE_INSP_HORZ_SHIFT_SPIN,
        IDC_SURFACE_INSP_ROTATE_SPIN, IDC_SURFACE_INSP_VERT_STRETCH_SPIN,
        IDC_SURFACE_INSP_HORZ_STRETCH_SPIN
    };
    // UDM_SETRANGE lParam: LOWORD = upper bound, HIWORD = lower bound (0).
    for ( int s : spins )
        SendDlgItemMessage( s, UDM_SETRANGE, 0, (LPARAM)1000 );
    SendDlgItemMessage( IDC_SURFACE_INSP_TEX_REP_X_SPIN, UDM_SETRANGE, 0, (LPARAM)1024 );
    SendDlgItemMessage( IDC_SURFACE_INSP_TEX_REP_Y_SPIN, UDM_SETRANGE, 0, (LPARAM)1024 );

    CheckDlgButton( IDC_SURFACE_INSP_PATCH_2D, 0 );
    CheckDlgButton( IDC_SURFACE_INSP_PATCH_3D, 0 );
    CheckDlgButton( IDC_SURFACE_INSP_PATCH_CURVE, 0 );

    if ( !g_qeglobals.surfInsp_tex_repeatx ) g_qeglobals.surfInsp_tex_repeatx = 1;
    if ( !g_qeglobals.surfInsp_tex_repeaty ) g_qeglobals.surfInsp_tex_repeaty = 1;
    char buf[32];
    _itoa( g_qeglobals.surfInsp_tex_repeatx, buf, 10 );
    SetDlgItemText( IDC_SURFACE_INSP_TEX_REP_X, buf );
    _itoa( g_qeglobals.surfInsp_tex_repeaty, buf, 10 );
    SetDlgItemText( IDC_SURFACE_INSP_TEX_REP_Y, buf );

    if ( !g_qeglobals.surfInsp_nIDButton )
        g_qeglobals.surfInsp_nIDButton = IDC_SURFACE_INSP_PATCH_2D;
    switch ( g_qeglobals.surfInsp_nIDButton )
    {
        case IDC_SURFACE_INSP_PATCH_2D:    CheckDlgButton( IDC_SURFACE_INSP_PATCH_2D, 1 );    break;
        case IDC_SURFACE_INSP_PATCH_3D:    CheckDlgButton( IDC_SURFACE_INSP_PATCH_3D, 1 );    break;
        case IDC_SURFACE_INSP_PATCH_CURVE: CheckDlgButton( IDC_SURFACE_INSP_PATCH_CURVE, 1 ); break;
    }

    m_texdefDirty = 0;
    m_sampleDirty = 0;
    return TRUE;
}

// EN_CHANGE handlers (0x458e10 / 0x458e20) — mark pending edits so GetTexMods commits them.
void CSurfaceDlg::OnTexdefEdited() { m_texdefDirty = 1; }
void CSurfaceDlg::OnSampleEdited() { m_sampleDirty = 1; }

// OnOK (Done 1489 / Enter, 0x4589e0): commit pending edits (GetTexMods), then close.  This is
// DestroyWindow fires OnDestroy (g_surfwin=0) then PostNcDestroy (g_pSurfDlg=0,
// delete this).  Do NOT call CDialog::OnOK (it ::EndDialog-hides a modeless dialog).
void CSurfaceDlg::OnOK()
{
    SI_GetTexMods( this );
    SurfaceInspector_Wnd02();   // commit the scratch back (SurfaceDlg_Wnd02 0x4583f0)
    g_surfwin = 0;
    DestroyWindow();
}

// OnCancel (Cancel 1198 / Esc / [X], sub_458AB0): restore the template + flush pending dirty
// face/patch edits (Wnd02) WHILE g_surfwin is still set, then really close.  Do NOT call
// CDialog::OnCancel (it ::EndDialog-hides a modeless dialog, leaving g_surfwin/g_pSurfDlg set).
void CSurfaceDlg::OnCancel()
{
    SurfaceInspector_Wnd02();
    g_surfwin = 0;
    DestroyWindow();
}

// The two on-screen buttons route to the real close handlers above.
void CSurfaceDlg::OnDone()    { OnOK();     }   // "Done"   button 1489
void CSurfaceDlg::OnCancel2() { OnCancel(); }   // "Cancel" button 1198

// Title-bar [X] (WM_CLOSE → SurfaceInspector::OnClose2 0x458a10): g_surfwin=0 +
// CWnd::OnClose ONLY — the binary does NOT commit pending texmods on [X]; the Wnd02
// flush belongs to the Cancel button path (1198, 0x458ab0) alone (decoded msgmap
// 0x6E2E58).  [audit U8/D6 — the port routed [X] to OnCancel, silently COMMITTING
// scratch texdef edits the binary discards.]
void CSurfaceDlg::OnClose()
{
    g_surfwin = 0;                  // 0x458a10
    DestroyWindow();                // modeless port-form CWnd::OnClose (fires OnDestroy/PostNcDestroy)
}

// "Fit" (SurfaceInspector::OnFit 0x458DE0): fit the texture to the selected face(s) —
// Brush_FitTexture(1,1,0) computes the texdef so the texture tiles once across the face,
// then invalidate every view (camera re-projects via Face_MoveTexture) and re-read the fields.
void CSurfaceDlg::OnFit()
{
    Brush_FitTexture( 1.0f, 1.0f, 0 );
    g_nUpdateBits = -1;
    Surf_RefreshFields();
}

// Read a tex-repeat edit field as a float (0 if empty / unparseable).
static float SI_ReadEditFloat( HWND self, int id )
{
    char buf[128] = "";
    ::GetDlgItemTextA( self, id, buf, sizeof( buf ) - 1 );
    return (float)atof( buf );
}

// "Natural" (SurfaceInspector::OnNaturalize 0x458C60): patch 1:1 natural projection at the
// layer's default sample size (Select_SetTexture).  No-op when no patch is selected
// (Patch_NaturalizeSelected scans for the first selected patch).
void CSurfaceDlg::OnNaturalize()
{
    float x[2];
    Select_SetTexture( x );
    Patch_NaturalizeSelected( 0, 0, x[0], x[1] );
    g_nUpdateBits = -1;
}

// "CAP" (SurfaceInspector::OnCap 0x458B40): patch cap texturing — scale the layer sample
// size by the tex-repeat X/Y fields, then Patch_NaturalizeSelected(unk=1).  Bails (matching
// the binary) if either tex-repeat reads 0.
void CSurfaceDlg::OnCap()
{
    float v4[2];
    Select_SetTexture( v4 );
    float xSize = SI_ReadEditFloat( GetSafeHwnd(), IDC_SURFACE_INSP_TEX_REP_X );
    if ( xSize != 0.0f )
    {
        xSize *= v4[0];
        float ySize = SI_ReadEditFloat( GetSafeHwnd(), IDC_SURFACE_INSP_TEX_REP_Y );
        if ( ySize != 0.0f )
        {
            ySize *= v4[1];
            Patch_NaturalizeSelected( 1, 0, xSize, ySize );
            g_nUpdateBits = -1;
        }
    }
}

// "Lmap" (SurfaceInspector::OnLightmap 0x458CA0): re-align the lightmap layer's texCoords
// across the selected patches (Patch_Lightmap_Texturing).
void CSurfaceDlg::OnLightmap()
{
    Patch_Lightmap_Texturing();
    g_nUpdateBits = -1;
}

// "Set..." (SurfaceInspector::OnSet 0x458CB0): read the tex-repeat X/Y + the terrain-distance
// (2D/3D/Curve) mode radio and texture the selected patches (Patch_SetTexturing).  Caches the
// chosen mode + repeats into g_qeglobals for the next open (OnInitDialog restores them).
void CSurfaceDlg::OnSet()
{
    HWND self = GetSafeHwnd();
    float v5 = SI_ReadEditFloat( self, IDC_SURFACE_INSP_TEX_REP_X );
    float v6 = SI_ReadEditFloat( self, IDC_SURFACE_INSP_TEX_REP_Y );

    int checked = 0;                         // GetCheckedRadioButton(2D..Curve)
    if ( IsDlgButtonChecked( IDC_SURFACE_INSP_PATCH_2D ) )         checked = IDC_SURFACE_INSP_PATCH_2D;
    else if ( IsDlgButtonChecked( IDC_SURFACE_INSP_PATCH_3D ) )    checked = IDC_SURFACE_INSP_PATCH_3D;
    else if ( IsDlgButtonChecked( IDC_SURFACE_INSP_PATCH_CURVE ) ) checked = IDC_SURFACE_INSP_PATCH_CURVE;

    if ( checked )
        g_qeglobals.surfInsp_nIDButton = checked;
    g_qeglobals.surfInsp_tex_repeatx = (int)v5;
    g_qeglobals.surfInsp_tex_repeaty = (int)v6;

    switch ( checked )
    {
        case IDC_SURFACE_INSP_PATCH_2D:    Patch_SetTexturing( v5, v6, 0 ); break;
        case IDC_SURFACE_INSP_PATCH_3D:    Patch_SetTexturing( v5, v6, 1 ); break;
        case IDC_SURFACE_INSP_PATCH_CURVE: Patch_SetTexturing( v5, v6, 2 ); break;
        default: Sys_Printf( "Select a texturing mode for SET\n" ); break;
    }
    g_nUpdateBits = -1;
}

// UDN_DELTAPOS (OnDeltaPosSpin 0x458b10) — a spin arrow stepped; step the texdef by the
// binary's per-spin delta (UpdateSpinners).  Up-arrow = iDelta > 0.
void CSurfaceDlg::OnDeltaPosSpin( NMHDR *pNMHDR, LRESULT *pResult )
{
    NMUPDOWN *ud = (NMUPDOWN *)pNMHDR;
    SI_UpdateSpinners( this, (int)pNMHDR->idFrom, ud->iDelta > 0 );
    if ( pResult )
        *pResult = 0;
}

// OnDestroy (0x458a50) — save the window rect + clear g_surfwin + invalidate the views.
// [audit U8/D2 — the rect save was missing (this comment already claimed it); binary
// order is save BEFORE CWnd::OnDestroy (0x458a69/0x458a7a → 0x458a84).]
extern BOOL SaveRegistryInfo( const char *pszName, void *pvBuf, int lSize );   // win_qe3.cpp
void CSurfaceDlg::OnDestroy()
{
    if ( GetSafeHwnd() )                                     // 0x458a62
    {
        RECT rect;
        ::GetWindowRect( GetSafeHwnd(), &rect );             // 0x458a69
        SaveRegistryInfo( "Radiant::SurfaceWindow", &rect, 0x10 );   // 0x458a7a
    }
    CDialog::OnDestroy();                                    // 0x458a84
    g_surfwin = 0;                                           // 0x458a89
    g_nUpdateBits = -1;                                      // 0x458a93
}

void CSurfaceDlg::PostNcDestroy()
{
    g_surfwin  = 0;
    g_pSurfDlg = nullptr;
    delete this;                 // modeless self-cleanup
}

// DoSurface (0x4585d0) — open/show the inspector.  Toggling: closed → CDialog::Create(IDD 116);
// open → bring to front + refresh.  Faithful to the binary (a modeless CDialog on IDD_SURFACE_INSP).
void Surf_OpenInspector()
{
    CWnd *parent = AfxGetMainWnd();

    if ( g_pSurfDlg && ::IsWindow( g_pSurfDlg->GetSafeHwnd() ) )
    {
        g_surfwin = (int)(intptr_t)g_pSurfDlg->GetSafeHwnd();
        Surf_RefreshFields();
        g_pSurfDlg->ShowWindow( SW_SHOW );
        g_pSurfDlg->SetForegroundWindow();
        SurfaceInspector_SetTexMods();   // DoSurface re-open path: re-snapshot the scratch
        return;
    }

    g_bNewFace = 1;
    // Seed g_patch_texdef defaults (DoSurface 0x4585d0: size/shift 0.05, rotate = prefs rotation).
    g_patch_texdef.def.mat_texDef.size[0]  = 0.05f;
    g_patch_texdef.def.mat_texDef.size[1]  = 0.05f;
    g_patch_texdef.def.mat_texDef.shift[0] = 0.05f;
    g_patch_texdef.def.mat_texDef.shift[1] = 0.05f;

    g_pSurfDlg = new CSurfaceDlg();
    if ( !g_pSurfDlg->Create( IDD_SURFACE_INSPECTOR, parent ) )
    {
        delete g_pSurfDlg;
        g_pSurfDlg = nullptr;
        return;
    }
    g_pSurfDlg->ShowWindow( SW_SHOW );
    g_surfwin = (int)(intptr_t)g_pSurfDlg->GetSafeHwnd();
    Surf_RefreshFields();
    SurfaceInspector_SetTexMods();   // DoSurface: snapshot the current edit-layer into the scratch
}

// The selection→inspector refresh hook (UpdateSurfaceDialog 0x458590) — re-snapshot the
// scratch (SetTexMods) + refresh the edit fields (Select_SetTexture_2) whenever the inspector
// is open and the selection / map / a live texdef edit changes.  Called from
// CMainFrame::UpdateWindows (the invalidation broadcast) and after a map load (map.cpp).
// The SetTexMods re-baseline keeps the per-face mtldef[3] scratch in sync with each applied
// edit, so the close-time SurfaceDlg_Wnd02 commit is a safe no-op in steady state.  A no-op
// when the inspector is not up (so headless / selftest is unchanged).
void Surf_UpdateInspector()
{
    // Only when the inspector is a live, visible window.  The IsWindow/IsWindowVisible guard is
    // defensive: a proper close now destroys the dialog (clearing g_surfwin/g_pSurfDlg), so this
    // is a true no-op once closed — but never operate on a hidden/dead HWND.
    if ( g_surfwin && g_pSurfDlg && ::IsWindow( g_pSurfDlg->GetSafeHwnd() )
         && g_pSurfDlg->IsWindowVisible() )
    {
        SurfaceInspector_SetTexMods();   // 0x458270 — re-snapshot (UpdateSurfaceDialog)
        Surf_RefreshFields();            // 0x4572D0 — refresh the dialog fields
    }
}
