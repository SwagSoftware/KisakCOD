#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Editor MaterialDef accessors and layer initialization.
// A valid MaterialDef owns exactly one layered-material or Radiant-material handle.

#include "stdafx.h"
#include "qe3.h"
#include <gfx_d3d/r_material.h>   // Material_CastsStencilShadow (0x4FEE90)
#include <map>
#include <string>
#include <cmath>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Local definition completing qe3.h's pointer-only declaration.
// ─────────────────────────────────────────────────────────────────────────────

// LayerMaterialDef — the lyrMtl target (= the 84-byte layered-material library
// entry, padded to the IDB's 120-byte in-memory type).  CORRECTION vs the IDB
// auto-type: offset 0 is the NAME string (Materialdef_GetName returns (char*)lyrMtl),
// NOT a qtexture_s* — the binary's LayeredMaterials_GetMaterial name-compares the
// entry as a string starting at offset 0.
struct LayerMaterialDef
{
    char  name[64];        // 0x00  material name (Materialdef_GetName reads this)
    int   _pad_40;         // 0x40
    int   layerCount;      // 0x44
    int   current_layer;   // 0x48
    int   _pad_4C;         // 0x4C  (MaterialDef_13 reads this + 8*visIndex)
    int   layers_ptr;      // 0x50  &layers[0]; each layer is 8 bytes {radMtl, ?}
    int   _tail[9];        // 0x54..0x77
};
static_assert(sizeof(LayerMaterialDef) == 120, "LayerMaterialDef must be 120 bytes (IDB)");

// MaterialDef realize-state (defined in engine_stubs.cpp). MaterialDef_05..09 AND
// per-texture flag bits into it; MaterialDef_02 walks the layers feeding them.
extern int dword_181F51C;

// Assert is not declared in the shared headers (matches filters.cpp's pattern).
extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// gfx_d3d/r_material.cpp — looks up a material constant (e.g. "colorTint") by name
// and copies its vec4 literal into the out vector (IDB 0x51AD50).  Pointer-only here
// (Material is forward-declared in qe3.h), so a forward decl is enough.
extern char __cdecl Material_GetConstantValue( Material *material, const char *name, float *outValue );

// ─────────────────────────────────────────────────────────────────────────────
// MtlDef_IsValid — the invariant asserted throughout: exactly one of lyrMtl/radMtl.
// (Inlined as the assert condition in the binary; factored here for clarity.)
// ─────────────────────────────────────────────────────────────────────────────
static inline bool MtlDef_IsValid( const MaterialDef *m )
{
    return m && ((m->lyrMtl != nullptr) + (m->radMtl != nullptr) == 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Headless layered-material intern table.
// One stable, zeroed LayerMaterialDef per unique name (name@0, layerCount=0).
// ─────────────────────────────────────────────────────────────────────────────
static std::map<std::string, LayerMaterialDef *> &DegenMtlTable()
{
    static std::map<std::string, LayerMaterialDef *> t;
    return t;
}
static LayerMaterialDef *MakeDegenerateLayerMtl( const char *name )
{
    std::string key = name ? name : "";
    auto it = DegenMtlTable().find( key );
    if ( it != DegenMtlTable().end() )
        return it->second;
    LayerMaterialDef *lm = new LayerMaterialDef();   // value-init → all zero
    // name[0..63]; CoD material names are < 64 chars (Texture_GetHandle warns at >=64)
    strncpy( lm->name, key.c_str(), sizeof(lm->name) - 1 );
    lm->name[sizeof(lm->name) - 1] = '\0';
    lm->layerCount    = 0;     // degenerate: GetLayeredMaterial→0, Init loops 0×
    lm->current_layer = 0;
    DegenMtlTable()[key] = lm;
    return lm;
}

// 0x4315C0  SetMaterial — assign a named material to a MaterialDef channel.
//
// KISAK: without a renderer, preserve names in zero-layer handles instead of registering
// render materials; GUI operation uses the binary dispatch below.
extern bool g_radiantFirstLightRendererReady;                 // mainfrm.cpp
extern void *LayeredMaterials_GetMaterial( const char *name );     // layeredmaterials.cpp 0x4172f0
extern qtexture_s *Texture_GetHandle( const char *name );     // texwnd.cpp 0x45a8e0

// ── Prefab-load material gate (prefab-load-on-open) ──────────────────────────────
// KISAK: prefab parsing also uses zero-layer handles because the shared engine's water
// material layout differs from CoD4. The counter supports nested prefab loads.
int g_radiantLoadingPrefab = 0;                               // bracketed by Eclass_RealizeModel

void SetMaterial( const char *tex_name, patchMesh_material *out )
{
    if ( g_radiantFirstLightRendererReady && g_radiantLoadingPrefab == 0 )
    {
        out->lyrMtl = (LayerMaterialDef *)LayeredMaterials_GetMaterial( tex_name );
        out->radMtl = out->lyrMtl ? nullptr : Texture_GetHandle( tex_name );
    }
    else
    {
        out->lyrMtl = MakeDegenerateLayerMtl( tex_name );     // headless gate / prefab-load fallback
        out->radMtl = nullptr;
    }
    if ( ((out->lyrMtl != nullptr) + (out->radMtl != nullptr)) != 1 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                65, 0, "%s", "MtlDef_IsValid( mtlDef )" );
}

// Materialdef_Realize — lazily upgrade a DEGENERATE material (built during headless / prefab
// load with MakeDegenerateLayerMtl: lyrMtl->layerCount==0, radMtl==0) to a REAL one, now that the
// renderer is up.  The degenerate lyrMtl stores the material NAME at offset 0 (Materialdef_GetName),
// so we re-run SetMaterial's real dispatch by name.  Returns true if the def now has a drawable
// material (layerCount>0 OR radMtl set).  Used by the FILLED PATCH draw (pmesh.cpp Patch_DrawFilled):
// mp_backlot/blackout terrain patches load as prefab CONTENTS (g_radiantLoadingPrefab != 0) so their
// materials came out degenerate; realizing them here textures the terrain.  A material that genuinely
// has no lit technique (tool/caulk) simply resolves to a radMtl with no lit tech and draws flat — the
// same as brush faces (no invented data).
bool Materialdef_Realize( MaterialDef *md )
{
    if ( !md )
        return false;
    // Already real: a radMtl handle, or a layered material with >0 layers.
    if ( md->radMtl )
        return true;
    if ( md->lyrMtl && md->lyrMtl->layerCount > 0 )
        return true;
    if ( !md->lyrMtl )
        return false;
    if ( !g_radiantFirstLightRendererReady )
        return false;                                     // still headless — cannot realize
    const char *name = (const char *)md->lyrMtl;          // degenerate lyrMtl @0 == name
    if ( !name || !*name )
        return false;
    LayerMaterialDef *real = (LayerMaterialDef *)LayeredMaterials_GetMaterial( name );
    if ( real )
    {
        md->lyrMtl = real;
        md->radMtl = nullptr;
    }
    else
    {
        qtexture_s *tex = Texture_GetHandle( name );
        if ( !tex )
            return false;
        md->lyrMtl = nullptr;
        md->radMtl = tex;
    }
    return ( md->radMtl != nullptr ) || ( md->lyrMtl && md->lyrMtl->layerCount > 0 );
}

// 0x431640  Materialdef_GetName — the material name (lyrMtl@0 or radMtl->name).
LayerMaterialDef *Materialdef_GetName( MaterialDef *mtlDef )
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x43165d test esi,esi + invariant (level 0)
    if ( mtlDef->lyrMtl )
        return mtlDef->lyrMtl;                             // (char*)lyrMtl == name
    return (LayerMaterialDef *)mtlDef->radMtl->name;       // radMtl->name
}

// 0x4314A0  MaterialDef_GetLayeredMaterial — the renderable texture/layer handle.
qtexture_s *MaterialDef_GetLayeredMaterial( MaterialDef *mtlDef )
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x4314a3 test esi,esi + invariant (level 0)
    if ( mtlDef->radMtl )
        return mtlDef->radMtl;
    if ( !mtlDef->lyrMtl )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                30, 1, "%s", "mtlDef->lyrMtl" );           // LEVEL 1 sanity -> KEEP_VERBOSE
    if ( mtlDef->lyrMtl->layerCount )
        return (qtexture_s *)(intptr_t)mtlDef->lyrMtl->layers_ptr;
    return nullptr;                                        // degenerate (0 layers)
}

// 0x431520  MaterialDef_02 — "realize": invoke cb on each layer's radMtl handle
// (radMtl path: once on the texture; lyrMtl path: once per layer).
void MaterialDef_02( MaterialDef *mtlDef, int (*cb)( qtexture_s * ) )
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x431523 test esi,esi + invariant (level 0)
    if ( mtlDef->radMtl )
    {
        cb( mtlDef->radMtl );
        return;
    }
    iassert( mtlDef->lyrMtl );                             // 0x431571 (level 0)
    // layers[] live at offset 80 (layers_ptr), 8-byte stride; layer[i].radMtl@+0.
    for ( int i = 0; i < mtlDef->lyrMtl->layerCount; ++i )
    {
        qtexture_s **layer = (qtexture_s **)( (char *)mtlDef->lyrMtl + 80 + 8 * i );
        cb( *layer );
    }
}

// 0x431740  MaterialDef_04 / 0x431AB0 MaterialDef_11 — layer count (radMtl → 1).
int MaterialDef_04( MaterialDef *mtlDef )
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x43175d (level 0)
    if ( mtlDef->lyrMtl )
        return mtlDef->lyrMtl->layerCount;
    if ( !mtlDef->radMtl )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                113, 1, "%s", "mtlDef->radMtl" );           // LEVEL 1 sanity -> KEEP_VERBOSE
    return 1;
}
int MaterialDef_11( MaterialDef *mtlDef )   // 0x431AB0 — identical to MaterialDef_04
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x431acd (level 0)
    if ( mtlDef->lyrMtl )
        return mtlDef->lyrMtl->layerCount;
    if ( !mtlDef->radMtl )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                264, 1, "%s", "mtlDef->radMtl" );           // LEVEL 1 -> KEEP_VERBOSE
    return 1;
}

// 0x431840..0x431A00  MaterialDef_05..09 — realize callbacks for MaterialDef_02.
// Each ANDs a per-texture flag from the qtexture handle into dword_181F51C.
int MaterialDef_05( qtexture_s *radMtl )   // tex_num   @0x10
{
    iassert( radMtl );                                     // 0x431849 (level 0)
    dword_181F51C &= radMtl->tex_num_or_localefilter;
    return dword_181F51C;
}
int MaterialDef_06( qtexture_s *radMtl )   // low word of unk_flags2 @0x0C
{
    iassert( radMtl );                                     // 0x4318a9 (level 0)
    dword_181F51C = (unsigned short)( (unsigned short)radMtl->unk_flags2 & dword_181F51C );
    return dword_181F51C;
}
int MaterialDef_07( qtexture_s *radMtl )   // unk1 byte @0x09
{
    iassert( radMtl );                                     // 0x431919 (level 0)
    dword_181F51C = (unsigned char)( (unsigned char)radMtl->unk1 & dword_181F51C );
    return dword_181F51C;
}
int MaterialDef_08( qtexture_s *radMtl )   // in_use @0x20
{
    iassert( radMtl );                                     // 0x431999 (level 0)
    dword_181F51C &= radMtl->in_use;
    return dword_181F51C;
}
int MaterialDef_09( qtexture_s *radMtl )   // color_filter @0x1C
{
    iassert( radMtl );                                     // 0x431a09 (level 0)
    dword_181F51C &= radMtl->color_or_surfacetype_filter;
    return dword_181F51C;
}

// 0x431B30  LayerMat::GetCurrentLayer — active layer index (radMtl → 0).
namespace LayerMat
{
    int GetCurrentLayer( MaterialDef *mtlDef )
    {
        iassert( MtlDef_IsValid( mtlDef ) );               // 0x431b4d (level 0)
        if ( mtlDef->lyrMtl )
            return mtlDef->lyrMtl->current_layer;
        if ( !mtlDef->radMtl )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                    276, 1, "%s", "mtlDef->radMtl" );        // LEVEL 1 -> KEEP_VERBOSE
        return 0;
    }
}

// 0x431BA0  MaterialDef_13 — per-layer vis value (radMtl → 0 for visIndex 0).
int MaterialDef_13( int visIndex, MaterialDef *mtlDef )
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x431bbd (level 0)
    if ( mtlDef->lyrMtl )
    {
        if ( (unsigned)visIndex >= (unsigned)mtlDef->lyrMtl->layerCount )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                    287, 0, "visIndex doesn't index mtlDef->lyrMtl->layerCount\n\t%i not in [0, %i)",
                    visIndex, mtlDef->lyrMtl->layerCount );  // prose #expr -> KEEP_VERBOSE
        return *(int *)( (char *)mtlDef->lyrMtl + 76 + 8 * visIndex );  // &xx020 + 2*visIndex
    }
    if ( !mtlDef->radMtl )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                291, 1, "%s", "mtlDef->radMtl" );            // LEVEL 1 -> KEEP_VERBOSE
    vassert( (visIndex == 0), "(visIndex) = %i", visIndex );
    return 0;
}

// 0x431C60  MaterialDef_14 — the Material* handle for a layer (radMtl → radMtl->next).
Material *MaterialDef_14( unsigned int visIndex, MaterialDef *mtlDef )
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x431c7d (level 0)
    if ( mtlDef->lyrMtl )
    {
        if ( visIndex >= (unsigned)mtlDef->lyrMtl->layerCount )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                    303, 0, "visIndex doesn't index mtlDef->lyrMtl->layerCount\n\t%i not in [0, %i)",
                    visIndex, mtlDef->lyrMtl->layerCount );  // prose #expr -> KEEP_VERBOSE
        qtexture_s **layer = (qtexture_s **)( (char *)mtlDef->lyrMtl + 80 + 8 * visIndex );
        if ( !(*layer)->next )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                    304, 0, "%s", "mtlDef->lyrMtl->layers[visIndex].radMtl->handle" );  // member-name -> KEEP_VERBOSE
        return (*layer)->next;
    }
    if ( !mtlDef->radMtl )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                308, 1, "%s", "mtlDef->radMtl" );            // LEVEL 1 -> KEEP_VERBOSE
    iassert( visIndex == 0 );                                // 0x431d1d (level 0)
    if ( !mtlDef->radMtl->next )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                310, 0, "%s", "mtlDef->radMtl->handle" );    // member-name (port: next) -> KEEP_VERBOSE
    return mtlDef->radMtl->next;
}

// 0x431D70  MaterialDef_GetActiveLayerHandle (sub_431D70) — handle of the active layer.
int MaterialDef_GetActiveLayerHandle( MaterialDef *mtlDef )
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x431d90 (level 0)
    if ( mtlDef->lyrMtl )
    {
        int **layer = (int **)( (char *)mtlDef->lyrMtl + 80 + 8 * mtlDef->lyrMtl->current_layer );
        if ( !**layer )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                    321, 0, "%s", "mtlDef->lyrMtl->layers[mtlDef->lyrMtl->activeLayer].radMtl->handle" );  // member-name -> KEEP_VERBOSE
        return **layer;
    }
    if ( !mtlDef->radMtl )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                325, 1, "%s", "mtlDef->radMtl" );            // LEVEL 1 -> KEEP_VERBOSE
    if ( !mtlDef->radMtl->next )               // radMtl->next (the Material* handle) @0
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                326, 0, "%s", "mtlDef->radMtl->handle" );    // member-name (port: next) -> KEEP_VERBOSE
    return (int)(intptr_t)mtlDef->radMtl->next;
}

// 0x431E90  MaterialDef_15_Drawflag_Multiply — drawflag/multiply gate.
bool MaterialDef_15_Drawflag_Multiply( int drawFlags, MaterialDef *m )
{
    if ( (drawFlags & 0xC) == 0 )
        return true;
    if ( (drawFlags & 4) != 0 && (drawFlags & 8) != 0 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                348, 0, "%s",
                "!(drawFlags & DRAWFLAG_ONLY_MULTIPLY) || !(drawFlags & DRAWFLAG_SKIP_MULTIPLY)" );
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( m );
    int unk = lm ? (unsigned short)lm->unk_flags2 : 0;
    return ( drawFlags & ( 4 * ( (unk & 0x70) != 0x70 ) + 4 ) ) != 0;
}

// 0x431A60  MaterialDef_10_LayeredMatHandle — does the layered material's resolved
// render handle cast a stencil shadow?  Resolves the renderable layer/texture handle
// (MaterialDef_GetLayeredMaterial → qtexture_s), then forwards its registered Material*
// (lm->next @ offset 0) to Material_CastsStencilShadow (0x4FEE90, gfx_d3d/r_material.cpp).
// surfaceFlags is now carried on the KISAK_RADIANT-widened Material (populated in
// Material_LoadRaw from the on-disk MaterialInfoRaw.surfaceFlags), so the gate is
// byte-faithful to the IW3 56-byte MaterialInfo without disturbing SP/MP.
// NOTE: the only binary caller is Brush_DrawSubmitFaceWindings (0x47b380, brush.cpp P5.4
// TODO, unported), so this has no live caller in the port yet — it is correct and ready.
bool MaterialDef_10_LayeredMatHandle( MaterialDef *mtlDef )
{
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( mtlDef );   // 0x431a61
    if ( !lm )                                                   // 0x431a6a
        return false;
    if ( !lm->next )                                             // 0x431a70
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                246, 0, "%s", "radMtl->handle" );  // member-name (port: next) -> KEEP_VERBOSE
    return Material_CastsStencilShadow( lm->next );              // 0x431a96
}

// 0x431E40  MaterialDef_SetColorTint (sub_431E40) — reads the active
// layer's resolved Material handle (MaterialDef_14) and copies its "colorTint" vec4
// constant into out via Material_GetConstantValue (0x51AD50).  The third arg is a
// float[4] OUT vector (disasm push order: out, "colorTint", mtl → cdecl reverses).
// No callers in the binary; ported for completeness.  Returns 1 if the constant was
// found, 0 otherwise (Material_GetConstantValue's return).
char MaterialDef_SetColorTint( MaterialDef *mtlDef, unsigned int visIndex, float *out )
{
    Material *mtl = MaterialDef_14( visIndex, mtlDef );
    if ( !mtl )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                336, 0, "%s", "mtl" );                // member-name -> KEEP_VERBOSE
    return Material_GetConstantValue( mtl, "colorTint", out );
}

// 0x472C00  Init_MaterialLayer — seed each layer's mat_texDef from the material
// dimensions and a sample size.  a2 carries the sample-size FLOAT bit-pattern
// The binary reinterprets the pointer-sized argument as a float.
int Init_MaterialLayer( MaterialDef *a1, MaterialDef *a2 )
{
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( a1 );
    int width  = lm ? lm->width  : 512;
    int height = lm ? lm->height : 512;
    float sampleSize = *(float *)&a2;
    int n = MaterialDef_04( a1 );                 // layer count (degenerate → 0)
    int result = n;
    float *v6 = &a1->mat_texDef.size[1];          // each block is 7 floats (28 bytes)
    for ( int i = 0; i < n; ++i )
    {
        v6[-1] = (float)( width  * (double)sampleSize );
        v6[0]  = (float)( height * (double)sampleSize );
        v6[1]  = 0.0f;
        v6[2]  = 0.0f;
        v6[3]  = 0.0f;
        v6[4]  = 0.0f;
        result = MaterialDef_13( i, a1 );
        *(int *)&v6[5] = result;
        v6 += 7;
    }
    return result;
}

// 0x472DF0  TexMatToFakeTexCoords — canonicalise a lightmap_gray texdef (zero
// shift, fold rotate into [-45,45), abs sizes).  No-op for any other material.
void TexMatToFakeTexCoords( MaterialDef *mtlDef, texdef_sub_t *texDef )
{
    iassert( MtlDef_IsValid( mtlDef ) );                   // 0x472e1a test eax,eax + invariant (level 0)
    const char *name = mtlDef->lyrMtl ? (const char *)mtlDef->lyrMtl
                                      : mtlDef->radMtl->name;
    if ( !strcmp( name, "lightmap_gray" ) )
    {
        texDef->shift[0] = 0.0f;
        texDef->shift[1] = 0.0f;
        double rotate    = texDef->rotate;
        double snap      = floor( ( rotate + 45.0 ) / 90.0 ) * 90.0;
        texDef->rotate   = (float)( rotate - snap );
        texDef->size[0]  = (float)fabs( texDef->size[0] );
        texDef->size[1]  = (float)fabs( texDef->size[1] );
    }
}
