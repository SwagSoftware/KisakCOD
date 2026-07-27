#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\texwnd.cpp
//
// P5.4: the MATERIAL-REGISTRATION SUBSET of TexWnd.cpp (the rest of CTexWnd — the
// texture-browser window, palette draw, scroll/click — is P5.6). These three
// functions are the back half of SetMaterial's real dispatch (materialdef.cpp):
// a named material → a cached qtexture_s wrapping a registered engine Material*.
//   Texture_GetHandle (0x45a8e0)      — name → cached qtexture_s (registers if new)
//   Register_WorldMaterial (0x45a7b0) — name → engine Material* ("wc/<name>")
//   Editor_AddRadiantMaterial (0x45a5b0) — alloc + fill a qtexture_s, add to the list
//
// CoD3/CoD4 DIVERGENCE ADAPTATION (P3 ruling — kisak engine is authoritative):
// the binary reads autoTexScaleWidth/contents/surfaceFlags/usage/locale out of a CoD4
// 56-byte MaterialInfo that kisak's runtime MaterialInfo (0x18) does not retain. The
// editor-filter fields (usage/locale/surfaceFlags) are stashed on the editor-widened
// Material in Material_LoadRaw (editorUsage/editorLocale/surfaceFlags, same pattern as
// the existing surfaceFlags field), so Editor_AddRadiantMaterial fills the qtexture's
// usage_index/tex_num_or_localefilter/color_or_surfacetype_filter and TexWnd_FilterAccept's
// usage/locale/surfaceType browser filter (layer-0 LABEL_37) works on real data.  width/
// height come from the material's colormap image. Register_WorldMaterial still follows
// the binary's wc/<name> then optional wc/<name>_editor registration path.

#include "stdafx.h"
#include "qe3.h"
#include "prefs.h"                  // g_PrefsDlg->m_nTextureWindowScale / m_bTextureScrollbar

#include <gfx_d3d/r_material.h>
#include <gfx_d3d/r_gfx.h>
#include <gfx_d3d/r_font.h>         // Font_s::pixelHeight (TexWnd_SetupIter)
#include <universal/com_files.h>    // FS_ListFiles/FS_Read/FS_FOpenFileRead — bulk material load

// ── radiant-local helpers (home files) ────────────────────────────────────────
extern char *AllocMaterialString( const char *src );   // qe3.cpp     (0x48b030)
extern int   Sys_Printf( const char *fmt, ... );        // win_qe3.cpp (0x499e90)
extern char *va( const char *fmt, ... );                // q_shared.cpp (0x4B9850)
extern void  MainFrm_SetStatusText( int pane, const char *text ); // mainfrm.cpp (status pane sink)
extern int   g_nUpdateBits;                             // 0x25D5A74   (mainfrm.cpp)
// Assert is not declared in the shared headers (matches filters.cpp/materialdef.cpp).
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );

// ─────────────────────────────────────────────────────────────────────────────
// texWndGlob_textureOffset (IDB 0x25d7990, texwnd_s, 0x1006c bytes).
// Only the registration-relevant prefix is named here; the CTexWnd window state
// in the tail (m_nWidth/m_ptDown/nPos/…) is P5.6. Offsets match the IDB exactly so
// P5.6 can flesh out the tail without moving qtextures/sorted_materials/count.
// ─────────────────────────────────────────────────────────────────────────────
// nPos_s (IDB, 16 bytes) — per-(simple|layered) texture-window scroll state.
struct nPos_s
{
    int nPos_current;          // 0x00  scroll offset (px)
    int nPos_max;              // 0x04  content extent (scrollbar range)
    int nPos_layered_current;  // 0x08  (the layered-material sub-view — parked)
    int nPos_layered_max;      // 0x0c
};

struct texwnd_s
{
    int          textureOffset;                 // 0x00
    int          usageCount;                    // 0x04
    int          localeCount;                   // 0x08
    // IDB types these three `bool` but the filter handlers (e.g. 0x45b459
    // `mov ...usageFilter, bl`) store the FULL index byte (0..count-1), with an assert
    // that it fits in a byte — they are 1-byte INDEX fields, not booleans.  Same 1-byte
    // layout; type widened to uint8_t so `usageFilter + 60000` recovers the menu item id.
    unsigned char usageFilter;                  // 0x0c  (usage  filter index)
    unsigned char localeFilter;                 // 0x0d  (locale filter index)
    unsigned char surfaceTypeFilter;            // 0x0e  (surface-type filter index)
    bool         searchbar_filter;              // 0x0f
    const char  *searchbar_buffer;              // 0x10
    int          materialCount;                 // 0x14
    qtexture_s  *sorted_materials[16384];       // 0x18   (0x10000 bytes)
    qtexture_s  *qtextures;                      // 0x10018 (linked-list head, via ->prev)
    int          unk_8;                          // 0x1001c
    bool         m_bNeedRange;                   // 0x10020  scrollbar range valid
    char         _pad10021[3];                   // 0x10021
    int          m_nWidth;                       // 0x10024  pane client width  (px)
    int          m_nHeight;                      // 0x10028  pane client height (px)
    int          lastButtonDown;                 // 0x1002c
    int          m_ptDown[2];                    // 0x10030  CPoint
    bool         m_was_mouse_dragged;            // 0x10038
    bool         unk_bool;                        // 0x10039  (layered-material wnd active)
    char         _pad1003a[2];                   // 0x1003a
    nPos_s       nPos[3];                         // 0x1003c  per-layer scroll (current/max)
};
static_assert(sizeof(texwnd_s) == 0x1006c, "texwnd_s must match the IDB (0x1006c)");

texwnd_s texWndGlob_textureOffset = {};

// Head of the registered-material list (texWndGlob.qtextures @0x10018), walked via
// ->prev.  texwnd_s is TU-local, so out-of-file walkers (Map_LoadFile's in-use clear)
// go through this accessor.
qtexture_s *TexWnd_GetMaterialListHead() { return texWndGlob_textureOffset.qtextures; }

// Accessor for filters.cpp case-6 `Misc decal` seed (texWndGlob.unk_8 @0x1001c).  FillTextureMenu
// (now ported) sets this to 1<<localeIndex("decal") from locale.txt — with the stock locale.txt
// ("case","test","tools","decal",...) "decal" is locale index 3, so unk_8 = 1<<3 = 8.  (kisak's
// runtime MaterialInfo still drops the per-material locale bits the predicate ANDs against, so the
// seed is now faithful even though the data it filters is degenerate.)
int TexWnd_GetDecalLocaleBit() { return texWndGlob_textureOffset.unk_8; }

// Loaded usage/locale filter counts (texwnd_s is TU-local) — used by CMainFrame::OnCreate's
// FillTextureMenu logging.
int texWndGlob_textureOffset_usageCount()  { return texWndGlob_textureOffset.usageCount; }
int texWndGlob_textureOffset_localeCount() { return texWndGlob_textureOffset.localeCount; }

// Layered-material window "live add" flag (texwnd_s.unk_bool @0x10039).  The layered-
// material tool palette (layeredmaterialwnd.cpp sub_417440 / LayeredMaterials_AddEntries)
// sets this so the texture-window click-apply path can add a layer instead of replacing
// the face material.  Accessor avoids exporting the TU-local texwnd_s.
extern "C" void TexWnd_SetLayeredMaterialActive( int active )
{
    texWndGlob_textureOffset.unk_bool = ( active != 0 );
}

// ─────────────────────────────────────────────────────────────────────────────
// Editor material auto-texture scale: the binary uses the CoD4 MaterialInfo's
// autoTexScaleWidth/Height (≈ the colormap resolution). kisak does not retain it,
// so derive it from the material's first valid colormap image. Fallback 512.
// ─────────────────────────────────────────────────────────────────────────────
static int Radiant_OriginalImageDim( uint16_t dim, const GfxImage *img )
{
    int shift = img ? img->picmip.platform[0] : 0; // PICMIP_PLATFORM_USED
    int original = (int)dim;
    while ( shift-- > 0 && original < 32768 )
        original <<= 1;
    return original > 0 ? original : 512;
}

static void Radiant_MaterialTexScale( Material *mtl, int *outW, int *outH )
{
    *outW = 512;
    *outH = 512;
    if ( !mtl || !mtl->textureTable )
        return;

    // IDA's editor path reads autoTexScaleWidth/Height from the CoD4 material
    // header, i.e. the color-map texel density.  When we must fall back to the
    // runtime material, do not use textureTable[0]: the table is hash-sorted and
    // often starts with normalMap/specularMap.
    for ( int i = 0; i < mtl->textureCount; ++i )
    {
        if ( mtl->textureTable[i].semantic != 2 )
            continue;
        GfxImage *img = mtl->textureTable[i].u.image;
        if ( img && img->width && img->height )
        {
            *outW = Radiant_OriginalImageDim( img->width, img );
            *outH = Radiant_OriginalImageDim( img->height, img );
            return;
        }
    }

    for ( int i = 0; i < mtl->textureCount; ++i )
    {
        if ( mtl->textureTable[i].semantic == 11 )
            continue;
        GfxImage *img = mtl->textureTable[i].u.image;
        if ( img && img->width && img->height )
        {
            *outW = Radiant_OriginalImageDim( img->width, img );
            *outH = Radiant_OriginalImageDim( img->height, img );
            return;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Register_WorldMaterial (0x45a7b0) — register the editor world-material variant
// ("wc/<name>") and, when flagged, try the "_editor" world-material variant.
// ─────────────────────────────────────────────────────────────────────────────
struct RadiantMaterialInfo
{
    uint8_t  gameFlags;
    uint8_t  usage;
    uint16_t toolFlags;
    uint32_t locale;
    uint16_t autoTexScaleWidth;
    uint16_t autoTexScaleHeight;
    int      surfaceFlags;
    int      contents;
};

static bool Material_ReadEditorVariant( const char *editorName, MaterialInfoRaw *out );

static void Radiant_InfoFromRaw( const MaterialInfoRaw *raw, RadiantMaterialInfo *out )
{
    out->gameFlags          = raw->gameFlags;
    out->usage              = raw->usage;
    out->toolFlags          = raw->toolFlags;
    out->locale             = raw->locale;
    out->autoTexScaleWidth  = raw->autoTexScaleWidth;
    out->autoTexScaleHeight = raw->autoTexScaleHeight;
    out->surfaceFlags       = raw->surfaceFlags;
    out->contents           = raw->contents;
}

static void Radiant_InfoFromMaterialFallback( Material *mtl, const MaterialInfo *info, RadiantMaterialInfo *out )
{
    int w, h;
    Radiant_MaterialTexScale( mtl, &w, &h );
    out->gameFlags          = info ? info->gameFlags : 0;
    out->usage              = mtl ? mtl->editorUsage : 0;
    out->toolFlags          = 0;
    out->locale             = mtl ? mtl->editorLocale : 0;
    out->autoTexScaleWidth  = (uint16_t)w;
    out->autoTexScaleHeight = (uint16_t)h;
    out->surfaceFlags       = mtl ? mtl->surfaceFlags : 0;
    out->contents           = 0;
}

static void Radiant_ReadMaterialInfo( const char *name, Material *mtl,
                                      const MaterialInfo *runtimeInfo,
                                      RadiantMaterialInfo *outInfo )
{
    MaterialInfoRaw raw;
    if ( Material_ReadEditorVariant( name, &raw ) )
    {
        if ( raw.toolFlags & 0x1000 )
        {
            char editorName[80];
            strncpy( editorName, name, 64 );
            editorName[64] = '\0';
            I_strncat( editorName, sizeof( editorName ), "_editor" );

            MaterialInfoRaw editorRaw;
            if ( Material_ReadEditorVariant( editorName, &editorRaw ) )
                raw = editorRaw;
        }
        Radiant_InfoFromRaw( &raw, outInfo );
        return;
    }

    Radiant_InfoFromMaterialFallback( mtl, runtimeInfo, outInfo );
}

static Material *Register_WorldMaterial( const char *name, RadiantMaterialInfo *outInfo )
{
    char full[256];
    Com_sprintf( full, sizeof( full ), "wc/%s", name );
    Material *mtl = Material_RegisterHandle( full, 0 );
    MaterialInfo runtimeInfo;
    // IDA 0x45a7b0 never falls back to the base "<name>" material here.
    Material_GetInfo( mtl, &runtimeInfo );

    MaterialInfoRaw raw;
    if ( Material_ReadEditorVariant( name, &raw ) && ( raw.toolFlags & 0x1000 ) )
    {
        char editorName[80];
        strncpy( editorName, name, 64 );
        editorName[64] = '\0';
        I_strncat( editorName, sizeof( editorName ), "_editor" );

        char editorFull[256];
        Com_sprintf( editorFull, sizeof( editorFull ), "wc/%s", editorName );
        Material *editorMtl = Material_RegisterHandle( editorFull, 0 );
        if ( !Material_IsDefault( editorMtl ) )
        {
            mtl = editorMtl;
            Material_GetInfo( mtl, &runtimeInfo );
        }
    }

    Radiant_ReadMaterialInfo( name, mtl, &runtimeInfo, outInfo );
    return mtl;
}

// ─────────────────────────────────────────────────────────────────────────────
// Editor_AddRadiantMaterial (0x45a5b0) — allocate a qtexture_s, fill it from the
// registered Material*, and prepend it to texWndGlob's material list.
// ─────────────────────────────────────────────────────────────────────────────
static qtexture_s *Editor_AddRadiantMaterial( const RadiantMaterialInfo *mtlInfo,
                                              const char *name, Material *mtl )
{
    if ( !mtlInfo->autoTexScaleWidth || !mtlInfo->autoTexScaleHeight )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\TexWnd.cpp",
                209, 0, "%s", "mtlInfo->autoTexScaleWidth != 0 && mtlInfo->autoTexScaleHeight != 0" );

    qtexture_s *newRadMat = new qtexture_s();   // value-init → all zero (operator new in IDB)
    iassert(newRadMat);

    // The binary's Editor_AddRadiantMaterial (0x45a5b0) fills these from the embedded
    // CoD4 56-byte MaterialInfo (usage@+0x1C, locale@+0x20, surfaceFlags). kisak's 24B
    // runtime MaterialInfo drops usage/locale, so the editor build stashes them on the
    // registered Material* (editorUsage/editorLocale/surfaceFlags, populated in
    // Material_LoadRaw). Read them back here so this (map-material) path carries the same
    // browser-filter metadata as the bulk Load_Materials path (which reads the raw header
    // directly). A default/missing material has usage 0 → correctly filtered out, exactly
    // as the binary rejects usage_index==0 in TexWnd_IterateMaterials LABEL_37.
    newRadMat->next                        = mtl;                         // 0x00  THE rendered handle
    newRadMat->name                        = AllocMaterialString( name ); // 0x04
    newRadMat->is_in_use                   = true;                        // 0x08
    newRadMat->usage_index                 = (char)mtlInfo->usage;        // 0x0A  usage__0x1C
    newRadMat->tex_num_or_localefilter     = (int)mtlInfo->locale;        // 0x10  locale__0x20
    newRadMat->unk_flags2                  = (uint16_t)mtlInfo->toolFlags;// 0x0C  LOWORD(toolFlags)
    newRadMat->unk1                        = (char)mtlInfo->gameFlags;    // 0x09
    newRadMat->width                       = mtlInfo->autoTexScaleWidth;  // 0x14
    newRadMat->height                      = mtlInfo->autoTexScaleHeight; // 0x18
    newRadMat->color_or_surfacetype_filter = mtlInfo->surfaceFlags;       // 0x1C  surfaceFlags (full; the
                                                                          //       filter ANDs &0x1F00000)
    newRadMat->in_use                      = mtlInfo->contents;           // 0x20

    newRadMat->prev = texWndGlob_textureOffset.qtextures;                // 0x24  list link
    texWndGlob_textureOffset.qtextures = newRadMat;
    texWndGlob_textureOffset.sorted_materials[texWndGlob_textureOffset.materialCount++] = newRadMat;
    return newRadMat;
}

// ─────────────────────────────────────────────────────────────────────────────
// Texture_GetHandle (0x45a8e0) — name → cached qtexture_s. Walks the registered
// list (linked via ->prev); registers + adds on first reference. The found-with-
// null-handle branch lazily registers the engine material.
// ─────────────────────────────────────────────────────────────────────────────
qtexture_s *Texture_GetHandle( const char *name )
{
    if ( strlen( name ) >= 0x40 )
        Com_PrintMessage( "texture name '%s' is too long", name );

    char lowercaseName[256];
    strncpy( lowercaseName, name, sizeof( lowercaseName ) - 1 );
    lowercaseName[sizeof( lowercaseName ) - 1] = '\0';
    _strlwr( lowercaseName );
    iassert(lowercaseName[0] != '(');

    RadiantMaterialInfo info;

    for ( qtexture_s *q = texWndGlob_textureOffset.qtextures; q; q = q->prev )
    {
        if ( !_stricmp( lowercaseName, q->name ) )
        {
            bool needHandle = ( q->next == nullptr );
            q->is_in_use = true;
            if ( needHandle )
                q->next = Register_WorldMaterial( lowercaseName, &info );
            return q;
        }
    }

    // Not in the list — register the engine material and add a wrapper.
    Material *mtl = Register_WorldMaterial( lowercaseName, &info );
    if ( Material_IsDefault( mtl ) && I_stricmp( name, "$default" ) )
        Sys_Printf( "\t\t\tWARNING: Could not find material '%s'\n", name );
    return Editor_AddRadiantMaterial( &info, lowercaseName, mtl );
}

// ═════════════════════════════════════════════════════════════════════════════
//  BULK MATERIAL LOAD — populate the texture browser at editor startup with EVERY
//  material in the searchpaths, not just the ones the loaded map references.
//  Ported from IW3xRadiant.i64 (port 13343):
//    Load_Materials                   (0x45ae40) — FS_ListFilteredFiles("materials") loop
//    Material_ConvertToEditorMaterial (0x45aa30) — raw 0x28 header → editor qtexture
//    Material_IsEditorVariant         (0x45a6d0) — name ends with "_editor"
//    Editor_DoesMaterialExist         (0x45adb0) — dup check by name
//    Material_ReadEditorVariant       (0x45ade0) / Material_CopyEditorString (0x45a790)
//
//  Each material's on-disk 0x28 MaterialInfoRaw header carries autoTexScaleWidth/Height
//  + usage/locale/toolFlags/gameFlags/surfaceFlags/contents DIRECTLY — so unlike the
//  map-material path above (which has only the runtime Material*, missing those editor
//  fields), the qtexture's editor-filter fields are filled field-for-field exactly as
//  the binary's Editor_AddRadiantMaterial does.  The render handle (q->next) is left
//  NULL: DrawMaterials lazily binds it via Texture_GetHandle when the thumbnail scrolls
//  into view, so startup stays cheap (header read + alloc, no 4000+ material registrations).
// ═════════════════════════════════════════════════════════════════════════════

// 0x45a6d0 — does `name` end with the "_editor" world-craft suffix?
static bool Material_IsEditorVariant( const char *name )
{
    size_t n = strlen( name );
    if ( n < 7 )
        return false;
    return strcmp( name + n - 7, "_editor" ) == 0;
}

// 0x45adb0 — find an already-registered qtexture by name (case-insensitive walk).
static qtexture_s *Editor_DoesMaterialExist( const char *name )
{
    for ( qtexture_s *q = texWndGlob_textureOffset.qtextures; q; q = q->prev )
        if ( !_stricmp( name, q->name ) )
            return q;
    return nullptr;
}

// 0x45ade0 — open "materials/<editorName>" and read its 0x28 header into `out`.
static bool Material_ReadEditorVariant( const char *editorName, MaterialInfoRaw *out )
{
    int h = 0;
    com_fileAccessed = 1;
    FS_FOpenFileRead( va( "materials/%s", editorName ), &h );
    if ( !h )
        return false;
    uint32_t got = FS_Read( (uint8_t *)out, sizeof( MaterialInfoRaw ), h );
    FS_FCloseFile( h );
    return got == sizeof( MaterialInfoRaw );
}

// 0x45aa30 + 0x45a5b0 — raw header → browser qtexture (faithful field assignments,
// sourced from MaterialInfoRaw).  Returns the existing entry if already registered.
static qtexture_s *Material_ConvertToEditorMaterial( const MaterialInfoRaw *raw,
                                                     const char *name )
{
    qtexture_s *existing = Editor_DoesMaterialExist( name );
    if ( existing )
        return existing;

    qtexture_s *q = new qtexture_s();                              // value-init → all zero
    q->next                        = nullptr;                      // 0x00  lazy render handle
    q->name                        = AllocMaterialString( name );  // 0x04
    q->is_in_use                   = true;                         // 0x08
    q->unk1                        = (char)raw->gameFlags;         // 0x09
    q->usage_index                 = (char)raw->usage;             // 0x0A
    q->unk_flags2                  = (uint16_t)raw->toolFlags;     // 0x0C  (LOWORD = toolFlags)
    q->tex_num_or_localefilter     = (int)raw->locale;            // 0x10
    q->width                       = raw->autoTexScaleWidth;       // 0x14
    q->height                      = raw->autoTexScaleHeight;      // 0x18
    q->color_or_surfacetype_filter = raw->surfaceFlags;           // 0x1C
    q->in_use                      = raw->contents;               // 0x20
    q->prev = texWndGlob_textureOffset.qtextures;                 // 0x24  list link
    texWndGlob_textureOffset.qtextures = q;
    texWndGlob_textureOffset.sorted_materials[texWndGlob_textureOffset.materialCount++] = q;
    return q;
}

// 0x45ae40 — enumerate every material in the searchpaths and register the renderable,
// editor-visible ones (valid usage/locale + a real auto-scale) into the browser.
void Load_Materials( void )
{
    if ( !fs_searchpaths )
    {
        Sys_Printf( "Load_Materials: filesystem not initialized\n" );
        return;
    }

    int          numMaterials = 0;
    const char **names = FS_ListFiles( "materials", "", FS_LIST_ALL, &numMaterials );
    int          added = 0;

    for ( int i = 0; i < numMaterials; ++i )
    {
        const char *name = names[i];
        if ( Material_IsEditorVariant( name ) )
            continue;

        MaterialInfoRaw raw;
        int             h = 0;
        com_fileAccessed = 1;
        FS_FOpenFileRead( va( "materials/%s", name ), &h );
        if ( !h )
            continue;
        uint32_t got = FS_Read( (uint8_t *)&raw, sizeof( raw ), h );
        FS_FCloseFile( h );
        if ( got != sizeof( raw ) )
            continue;

        // Binary's nested-non-zero gate: skip materials that aren't browsable
        // (no usage/locale or a degenerate auto-scale that would div-by-zero the grid).
        if ( !raw.usage || !raw.locale || !raw.autoTexScaleWidth || !raw.autoTexScaleHeight )
            continue;

        // toolFlags & 0x1000 → a separate "<name>_editor" header overrides this one.
        if ( raw.toolFlags & 0x1000 )
        {
            char editorName[80];
            strncpy( editorName, name, 64 );
            editorName[64] = '\0';
            I_strncat( editorName, sizeof( editorName ), "_editor" );
            MaterialInfoRaw editorRaw;
            if ( Material_ReadEditorVariant( editorName, &editorRaw ) )
                raw = editorRaw;
        }

        Material_ConvertToEditorMaterial( &raw, name );
        ++added;
    }

    FS_FreeFileList( names );
    Sys_Printf( "Load_Materials: registered %d of %d materials into the texture browser\n",
                added, numMaterials );
}

// ═════════════════════════════════════════════════════════════════════════════
//  Textures→Show In Use — mark/count the textures the
//  loaded map actually references, so the browser can show only them.
//  Ported faithfully from IW3xRadiant.i64 (port 13343):
//    tex_set_is_in_use (0x45B6E0) / Texture_ForName (0x45B760) / Texture_ShowInuse (0x45B850).
//  Also runs at the END of every map load (map.cpp Map_LoadEntities → Texture_ShowInuse),
//  which is why it must be real (the in-use flag drives the usage filter + the count line).
// ═════════════════════════════════════════════════════════════════════════════

extern selbrush_t active_brushes;                                // map.cpp (0x23F189C)
extern selbrush_t selected_brushes;                              // map.cpp (0x23F1864)
extern void       MaterialDef_02( MaterialDef *m, int (*cb)( qtexture_s * ) ); // materialdef.cpp (0x431520)

// 0x45B6E0 — the Texture_ForName per-MaterialDef realize callback: flag the qtexture
// in-use. (Binary returns the qtexture*; MaterialDef_02 discards it — return value unused.)
static int tex_set_is_in_use( qtexture_s *q )
{
    q->is_in_use = true;
    return 0;
}

// 0x45A4D0 — qsort comparator for sorted_materials[] (each entry is a qtexture_s*):
// case-insensitive by material name (qtexture_s.name @ +4).
static int __cdecl Tex_stricmp( const void *a1, const void *a2 )
{
    return _stricmp( ( *(qtexture_s **)a1 )->name, ( *(qtexture_s **)a2 )->name );
}

// 0x45B760 — Texture_ForName: walk a brush list and flag every referenced material's
// qtexture in-use (via MaterialDef_02 → tex_set_is_in_use). `layer` = the material
// channel (0 = base). Recurses into prefab child brush lists. Faithful to the disasm:
//   - fixedsize (point) entity → recurse owner->prefab->active_brushlist (prefab + 0x0C).
//   - else, patch brush       → flag &patch->texture[layer] (patchMesh_material stride 8).
//   - else, plain brush       → flag &def->faces[i].mtldef[layer] for the DEF faceCount
//     (the authoritative count; the binary reads def+0x40).
void Texture_ForName( selbrush_t *list, int layer )
{
    for ( selbrush_t *b = list->next; b && b != list; b = b->next )
    {
        brush_t *def = b->def;
        if ( *(int *)&def->owner->eclass->fixedsize )
        {
            // Point/bbox entity: descend into its prefab's brush list (if any).
            void *prefab = b->owner->prefab;        // entity_s.prefab @ 0x48
            if ( prefab )
                Texture_ForName( (selbrush_t *)( (char *)prefab + 0x0C ), layer );
        }
        else if ( def->patch )
        {
            // Patch: one MaterialDef per layer at &patch->texture[layer] (0x18 + 8*layer).
            MaterialDef_02( (MaterialDef *)( &def->patch->texture + layer ), tex_set_is_in_use );
        }
        else
        {
            for ( int i = 0; i < def->faceCount; ++i )
                MaterialDef_02( &def->faces[i].mtldef[layer], tex_set_is_in_use );
        }
    }
}

// 0x45B850 — Texture_ShowInuse: clear all in-use flags + the usage/locale/surface-type
// filters, re-flag the materials used by the active + selected brushes, re-sort the
// browser list, invalidate, and report the count.
LRESULT Texture_ShowInuse()
{
    int layer = g_qeglobals.current_edit_layer;
    texWndGlob_textureOffset.nPos[layer].nPos_current         = 0;
    texWndGlob_textureOffset.nPos[layer].nPos_layered_current = 0;
    texWndGlob_textureOffset.localeFilter      = 0;
    texWndGlob_textureOffset.usageFilter       = 0;
    texWndGlob_textureOffset.surfaceTypeFilter = 0;

    for ( qtexture_s *q = texWndGlob_textureOffset.qtextures; q; q = q->prev )
        q->is_in_use = false;

    SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                  (LPARAM)"Selecting active textures\n" );
    Texture_ForName( &active_brushes,   0 );
    Texture_ForName( &selected_brushes, 0 );

    qsort( texWndGlob_textureOffset.sorted_materials,
           texWndGlob_textureOffset.materialCount, sizeof( qtexture_s * ), Tex_stricmp );

    g_nUpdateBits |= W_TEXTURE;   // binary immediate 0x10

    int n = 0;
    for ( qtexture_s *q = texWndGlob_textureOffset.qtextures; q; q = q->prev )
        if ( q->is_in_use )
            ++n;
    return Sys_Printf( "%d textures in use\n", n );
}

// 0x45B730 — Texture_ShowAll: mark EVERY registered qtexture in-use, so the browser shows the
// full registered set rather than only the selection's.  The Textures->Show All path (Ctrl-A,
// cmd 32973).  Unlike Show In Use it does NOT re-filter or re-sort — it just un-hides everything.
LRESULT Texture_ShowAll()
{
    for ( qtexture_s *q = texWndGlob_textureOffset.qtextures; q; q = q->prev )
        q->is_in_use = true;
    g_nUpdateBits |= W_TEXTURE;   // binary `or g_nUpdateBits, 10h` (binary W_TEXTURE==0x10)
    return Sys_Printf( "Showing all textures...\n" );
}

// The Usage/Locale/Surface-type filter name→index tables the LABEL_37 chain reads.
// filter_usage_array/filter_locale_array (engine_stubs.cpp, loaded by FillTextureMenu);
// filter_surfacetype_array (defined below in this TU).  RadiantFilterEntry = IDB
// filter_material_t {char* name; int index} (8B); the menu/shutdown code below reuses it.
// Declared here (ahead of both the RADIANT_SELFTEST gate hooks and TexWnd_FilterAccept,
// all of which read these arrays).
struct RadiantFilterEntry { char *name; int index; };
static_assert(sizeof(RadiantFilterEntry) == 8, "filter_material_t must be 8 bytes (IDB)");
extern RadiantFilterEntry filter_usage_array[256];       // IDB 0x739F80
extern RadiantFilterEntry filter_locale_array[256];      // IDB 0x73A780
extern RadiantFilterEntry filter_surfacetype_array[29];  // IDB 0x73AF80 (this TU, below)

#ifdef RADIANT_SELFTEST
// Gate hook — run Texture_ShowInuse and report the in-use vs total qtexture counts so
// the headless gate can assert the deterministic effect (the in-use list-count change).
void Radiant_TestShowInUse( int *outInUse, int *outTotal )
{
    Texture_ShowInuse();
    int inUse = 0, total = 0;
    for ( qtexture_s *q = texWndGlob_textureOffset.qtextures; q; q = q->prev )
    {
        ++total;
        if ( q->is_in_use )
            ++inUse;
    }
    if ( outInUse ) *outInUse = inUse;
    if ( outTotal ) *outTotal = total;
}

// Forward (defined below): the per-material accept predicate + the flow iterator.
static bool TexWnd_FilterAccept( const qtexture_s *tex );

// Gate hook — exercise the data-INDEPENDENT filter sub-conditions of TexWnd_IterateMaterials
// (the Lightmap/Smoothing render-method layer sub-views + the search-bar name prefix).
// Self-contained + self-restoring: synthesizes a fixed set of named materials, appends them
// to texWndGlob.sorted_materials, then counts TexWnd_FilterAccept hits at each layer / with a
// search prefix.  Restores materialCount + current_edit_layer + searchbar so it leaves the
// editor state untouched for subsequent gates in the same process.
//
// Returns the four counts via out-params:
//   *outLayer0  = accepted on layer 0 (Material): all in-use synthetic mtls.
//   *outLayer1  = accepted on layer 1 (Lightmap): only "lightmap_*".
//   *outLayer2  = accepted on layer 2 (Smoothing): only "smoothing_*".
//   *outSearch  = accepted on layer 0 with searchbar="wc/floor" (prefix): only that one.
void Radiant_TestFilterSubviews( int *outLayer0, int *outLayer1, int *outLayer2, int *outSearch )
{
    // Fixed synthetic set (names are pre-lowercased, like Texture_GetHandle stores them).
    static const char *kNames[] = {
        "wc/floor", "wc/wall", "lightmap_gray", "lightmap_white", "smoothing_a", "$default"
    };
    const int kN = (int)( sizeof( kNames ) / sizeof( kNames[0] ) );

    // Snapshot the live texWndGlob state we mutate.
    const int   savedCount     = texWndGlob_textureOffset.materialCount;
    const int   savedLayer     = g_qeglobals.current_edit_layer;
    const bool  savedSearchOn  = texWndGlob_textureOffset.searchbar_filter;
    const char *savedSearchBuf = texWndGlob_textureOffset.searchbar_buffer;

    // Build the synthetic qtextures and append to sorted_materials[].
    qtexture_s *synth[16] = {};
    for ( int i = 0; i < kN; ++i )
    {
        qtexture_s *q = new qtexture_s();
        q->name      = kNames[i];
        q->width     = 64;
        q->height    = 64;
        // usage_index must be non-zero or the ported layer-0 chain (reject usage_index==0,
        // IDB 0x45bc70) drops the material before the name/search sub-views are reached.  A
        // real browsable material always has a usage; use 1 for every synthetic entry.
        q->usage_index = 1;
        // "$default" is NOT in use; everything else is (so the layer-0 base predicate
        // distinguishes the in-use subset from the dead entry).
        q->is_in_use = ( kNames[i][0] != '$' );
        synth[i] = q;
        texWndGlob_textureOffset.sorted_materials[texWndGlob_textureOffset.materialCount++] = q;
    }

    auto countAccepted = [&]() -> int {
        int n = 0;
        for ( int i = 0; i < kN; ++i )
            if ( TexWnd_FilterAccept( synth[i] ) )
                ++n;
        return n;
    };

    g_qeglobals.current_edit_layer = 0;
    texWndGlob_textureOffset.searchbar_filter = false;
    // layer 0 excludes lightmap_*/smoothing_* (0x45bc5f/0x45bc68) and usage_index==0 first, so
    // of the 6 synthetics only the two in-use wc/* (usage 1) remain (the 2 lightmap_ + 1
    // smoothing_ are prefix-excluded; $default is not-in-use).
    if ( outLayer0 ) *outLayer0 = countAccepted();        // expect 2 (wc/floor, wc/wall)

    g_qeglobals.current_edit_layer = 1;
    if ( outLayer1 ) *outLayer1 = countAccepted();        // expect 2 (lightmap_*)

    g_qeglobals.current_edit_layer = 2;
    if ( outLayer2 ) *outLayer2 = countAccepted();        // expect 1 (smoothing_a)

    g_qeglobals.current_edit_layer = 0;
    texWndGlob_textureOffset.searchbar_filter = true;
    texWndGlob_textureOffset.searchbar_buffer = "wc/floor";
    if ( outSearch ) *outSearch = countAccepted();        // expect 1 (wc/floor prefix)

    // Restore.
    texWndGlob_textureOffset.materialCount    = savedCount;
    g_qeglobals.current_edit_layer            = savedLayer;
    texWndGlob_textureOffset.searchbar_filter = savedSearchOn;
    texWndGlob_textureOffset.searchbar_buffer = savedSearchBuf;
    for ( int i = 0; i < kN; ++i )
        delete synth[i];
}

// Gate hook (texfilter) — exercise the DATA-DEPENDENT layer-0 usage/locale/surfaceType
// reject chain of TexWnd_FilterAccept (IDB 0x45bc70..0x45bcdd).  Synthesizes qtextures with
// varied usage_index / tex_num_or_localefilter (locale BITMASK) / color_or_surfacetype_filter
// (full surfaceFlags; the chain ANDs &0x1F00000), drives texWndGlob.usageFilter/localeFilter/
// surfaceTypeFilter, and reports the accept counts.  Self-restoring (materialCount, layer,
// the three filter indices, and the filter-array slots it pokes).
//
//   *outAll          = index 0 ("all") on every axis: accept all in-use with usage!=0.
//   *outUsageReject0 = a material with usage_index==0 is rejected even at index 0.
//   *outUsageFilter  = usageFilter index set: only the material whose usage matches.
//   *outLocaleFilter = localeFilter index set: only the material whose locale bit is set.
//   *outSurfFilter   = surfaceTypeFilter index set: only the material whose surface type matches.
void Radiant_TestFilterUsageLocale( int *outAll, int *outUsageReject0,
                                    int *outUsageFilter, int *outLocaleFilter,
                                    int *outSurfFilter )
{
    // Snapshot live state we mutate.
    const int  savedCount  = texWndGlob_textureOffset.materialCount;
    const int  savedLayer  = g_qeglobals.current_edit_layer;
    const unsigned char savedUF  = texWndGlob_textureOffset.usageFilter;
    const unsigned char savedLF  = texWndGlob_textureOffset.localeFilter;
    const unsigned char savedSTF = texWndGlob_textureOffset.surfaceTypeFilter;
    const RadiantFilterEntry savedUA1 = filter_usage_array[1];
    const RadiantFilterEntry savedLA1 = filter_locale_array[1];

    g_qeglobals.current_edit_layer            = 0;
    texWndGlob_textureOffset.usageFilter      = 0;
    texWndGlob_textureOffset.localeFilter     = 0;
    texWndGlob_textureOffset.surfaceTypeFilter= 0;

    // Define a couple of filter array slots (index 1) with known target values.
    // usage filter slot 1 selects usage_index == 5;  locale filter slot 1 selects locale bit 3.
    filter_usage_array[1].name  = (char *)"glass_usage";
    filter_usage_array[1].index = 5;
    filter_locale_array[1].name = (char *)"decal_locale";
    filter_locale_array[1].index = 3;                 // tests (1<<3) & locale-mask

    // Synthetic materials:
    //   A: usage 5, locale mask 1<<3, surfaceType = metal (0xD00000)   — the "match all filters" one
    //   B: usage 7, locale mask 1<<1, surfaceType = wood  (0x1500000)  — non-matching usage/locale/surf
    //   C: usage 0 (no metadata)                                        — must be rejected even at "all"
    qtexture_s *A = new qtexture_s(); A->name = "wc/metal_a"; A->width=64; A->height=64; A->is_in_use=true;
    A->usage_index = 5;  A->tex_num_or_localefilter = (1<<3); A->color_or_surfacetype_filter = 0xD00000;
    qtexture_s *B = new qtexture_s(); B->name = "wc/wood_b";  B->width=64; B->height=64; B->is_in_use=true;
    B->usage_index = 7;  B->tex_num_or_localefilter = (1<<1); B->color_or_surfacetype_filter = 0x1500000;
    qtexture_s *C = new qtexture_s(); C->name = "wc/raw_c";   C->width=64; C->height=64; C->is_in_use=true;
    C->usage_index = 0;  C->tex_num_or_localefilter = 0;      C->color_or_surfacetype_filter = 0;

    qtexture_s *synth[3] = { A, B, C };
    for ( int i = 0; i < 3; ++i )
        texWndGlob_textureOffset.sorted_materials[texWndGlob_textureOffset.materialCount++] = synth[i];

    auto countAccepted = [&]() -> int {
        int n = 0;
        for ( int i = 0; i < 3; ++i ) if ( TexWnd_FilterAccept( synth[i] ) ) ++n;
        return n;
    };

    // "all" default (all filter indices 0): A + B accepted (usage!=0, in-use); C rejected (usage 0).
    if ( outAll )          *outAll = countAccepted();                 // expect 2
    if ( outUsageReject0 ) *outUsageReject0 = TexWnd_FilterAccept( C ) ? 1 : 0; // expect 0

    // usageFilter -> slot 1 (usage 5): only A.
    texWndGlob_textureOffset.usageFilter = 1;
    if ( outUsageFilter ) *outUsageFilter = countAccepted();         // expect 1 (A)
    texWndGlob_textureOffset.usageFilter = 0;

    // localeFilter -> slot 1 (bit 3): only A (mask 1<<3); B has 1<<1.
    texWndGlob_textureOffset.localeFilter = 1;
    if ( outLocaleFilter ) *outLocaleFilter = countAccepted();       // expect 1 (A)
    texWndGlob_textureOffset.localeFilter = 0;

    // surfaceTypeFilter -> "metal" (index 17 in filter_surfacetype_array, value 0xD00000): only A.
    texWndGlob_textureOffset.surfaceTypeFilter = 17;
    if ( outSurfFilter ) *outSurfFilter = countAccepted();           // expect 1 (A)
    texWndGlob_textureOffset.surfaceTypeFilter = 0;

    // Restore.
    texWndGlob_textureOffset.materialCount     = savedCount;
    g_qeglobals.current_edit_layer             = savedLayer;
    texWndGlob_textureOffset.usageFilter       = savedUF;
    texWndGlob_textureOffset.localeFilter      = savedLF;
    texWndGlob_textureOffset.surfaceTypeFilter = savedSTF;
    filter_usage_array[1]  = savedUA1;
    filter_locale_array[1] = savedLA1;
    for ( int i = 0; i < 3; ++i )
        delete synth[i];
}

// Gate hook (layeredmat) — pre-seed the Texture_GetHandle cache so the layered-material
// LOADER can be exercised headless.  In the GUI, QE_LoadProject runs Load_Textures()
// BEFORE LayerdMatWnd(), so every layer material the library references is already a
// registered qtexture in texWndGlob.qtextures with a live render handle — and
// LayeredMaterialLibrary's per-layer Texture_GetHandle therefore HITS the cache (texwnd.cpp
// ~204) and never reaches the renderer/FS-dependent Register_WorldMaterial.  The headless
// selftest has no renderer/FS, so we reproduce exactly that cache state here: push a
// qtexture for `name` (pre-lowercased, as Texture_GetHandle stores) with a NON-NULL `next`
// (so Texture_GetHandle's needHandle test is false → it returns this entry as-is).  This
// invents no behaviour — it sets up the same runtime state the GUI already has.
extern "C" void Radiant_TestSeedLayerMaterial( const char *loweredName )
{
    qtexture_s *q = new qtexture_s();
    q->next      = (Material *)(intptr_t)1;    // sentinel non-NULL → no lazy Register_WorldMaterial
    q->name      = AllocMaterialString( loweredName );
    q->is_in_use = true;
    q->width     = 64;
    q->height    = 64;
    q->prev      = texWndGlob_textureOffset.qtextures;
    texWndGlob_textureOffset.qtextures = q;
}
#endif

// ═════════════════════════════════════════════════════════════════════════════
// CTexWnd renders registered materials as a wrapping, scrollable thumbnail grid.
// Editor_AddRadiantMaterial and Material_ConvertToEditorMaterial populate the metadata
// consumed by its usage, locale, surface-type, and layer filters.
// ═════════════════════════════════════════════════════════════════════════════

#include "mainfrm.h"                  // CTexWnd
#include <gfx_d3d/r_init.h>           // dx, R_SetupRendertarget_CheckDevice, R_Hwnd_Resize, R_CheckTargetWindow
#include <gfx_d3d/r_rendercmds.h>     // R_BeginFrame/EndFrame, clear, ProjectionSet2D, Draw2DImage, DrawText

extern void  R_SortMaterials();                                   // r_ed_scene.cpp
extern void  Brush_SetTexture( MaterialDef *a1, char a3 );        // select.cpp (0x48f170)
extern char  Texture_SetTexture( const int *a1, MaterialDef *a2 ); // texwnd.cpp 0x45be50
extern qtexture_s *MaterialDef_GetLayeredMaterial( MaterialDef *mtlDef ); // materialdef.cpp 0x4314a0
extern int   MaterialDef_04( MaterialDef *mtlDef );              // materialdef.cpp 0x431740
extern int   MaterialDef_13( int visIndex, MaterialDef *mtlDef );// materialdef.cpp 0x431ba0
extern int   g_nUpdateBits;                                       // 0x25D5A74
// Texture_SetTexture deps (the back half of the function — declared here, defined further down):
extern CFindTextureDlg *g_dlgFind;                                // findtexture.cpp (0x73C700) — the find/replace dialog singleton
extern char  byte_73C380;                                        // findtexture.cpp (0x73C380) — 1=fill Find, 0=fill Replace on pick
extern LRESULT LayeredMaterialWnd_RadMtl( qtexture_s *radMtl );   // layeredmaterialwnd.cpp (0x4185C0) — add radMtl as a live layer
extern selface_t *selFace;                                        // select.cpp 0x73C710
extern char  g_ptrSelectedFaces_GetSize[4];                       // *(int*) = selected-face count
extern void  sub_477D70( selbrush_t *b, const float *mat );       // brush.cpp Brush_CheckBuildFaceVis
extern float world_orient_matrix[4][3];                           // 0x6DE290

// Layered-material sub-view (defined below; the R_DrawTexWnd 0x45d0f0 dispatch in OnPaint
// selects it via this mode flag).  See the LAYERED-MATERIAL SUB-VIEW block near EOF.
extern int  g_texwnd_simple_layered_selection;                    // IDB 0x25E79FC (defined below)
static int  TexWnd_DrawLayeredMaterials();                        // IDB 0x45d080

static void TexWnd_BuildClickedMaterialDef( qtexture_s *q, MaterialDef *out )
{
    memset( out, 0, sizeof( *out ) );
    out->radMtl = q;

    qtexture_s *layered = MaterialDef_GetLayeredMaterial( out );
    const int width = layered ? layered->width : 512;
    const int height = layered ? layered->height : 512;
    const int layerCount = MaterialDef_04( out );
    const float sample = g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].sampleSize;

    for ( int i = 0; i < layerCount; ++i )
    {
        texdef_sub_t *td = &out->mat_texDef + i;
        td->size[0] = (float)width * sample;
        td->size[1] = (float)height * sample;
        td->shift[0] = 0.0f;
        td->shift[1] = 0.0f;
        td->rotate = 0.0f;
        td->unk3 = 0;
        *(int *)&td->sample_size = MaterialDef_13( i, out );
    }
}

// ── grid layout constants (pane pixel space) ─────────────────────────────────
static const int TEX_MARGIN = 8;     // = the IDB's left/top start (thats_8 = 8)
// IDB: the per-thumbnail label band = font->pixelHeight (TexWnd_SetupIter 0x45ba00) and
// the name label is drawn at scale 1.0 in colors[8] (TexWnd_DrawMaterials 0x45ce57). A
// prior kisak adaptation used a compact fixed band + 0.18 text scale, which rendered the
// labels far too small to read.
static void R_DrawOutlineRect( int xL, int yT, int xR, int yB, const float *rgba );  // fwd (def below)

// ── MaterialIter_t (IDB, 44 bytes) — the flow-layout iterator. Field names mirror the
//    IDB offsets (gap8 → thumbX, dwordC → rowY, thats_8 → cursorX, thats_8_too → cursorY,
//    gap18[0]/[4] → thumbW/thumbH, dword20 → rowMaxH). ──────────────────────────────
struct MaterialIter_t
{
    qtexture_s *radMtl;            // 0x00  current material
    int         sortedIndex;       // 0x04  index into sorted_materials (init -1)
    int         thumbX;            // 0x08  laid-out x of this thumbnail
    int         rowY;              // 0x0c  laid-out y of this thumbnail (UNSCROLLED)
    int         cursorX;           // 0x10  running x within the current row
    int         cursorY;           // 0x14  running y of the current row top
    int         thumbW;            // 0x18  this thumbnail's width  (px)
    int         thumbH;            // 0x1c  this thumbnail's height (px)
    int         rowMaxH;           // 0x20  tallest thumbnail in the current row
    int         labelBand;         // 0x24  text band height (IDB font_pixelheight)
    float       texwndscale;       // 0x28  m_nTextureWindowScale / 100
};

// Ed_RoundToInt (IDB sub_40A7D0) — (int)(x + 2^-30). cvttsd2si truncates; the epsilon
// only nudges values sitting exactly on an integer boundary. Faithful.
static inline int Ed_RoundToInt( float x ) { return (int)( x + 9.313225746154785e-10f ); }

// TexWnd_SetupIter (IDB 0x45ba00).
static int TexWnd_SetupIter( MaterialIter_t *it, Font_s *font )
{
    it->cursorX      = TEX_MARGIN;
    it->cursorY      = TEX_MARGIN;
    it->sortedIndex  = -1;
    it->rowY         = 0;
    it->rowMaxH      = 0;
    it->texwndscale  = (float)( (double)g_PrefsDlg->m_nTextureWindowScale / 100.0 );
    it->labelBand    = font->pixelHeight;   // IDB 0x45ba00: label band = font->pixelHeight
    return it->labelBand;
}

// TexWnd_FilterAccept — the per-material accept predicate (IDB 0x45ba70 LABEL_37),
// ported 1:1 from the disasm.  Dispatches on g_qeglobals.current_edit_layer:
//   layer 1 (Lightmap render-method): accept iff the name starts with "lightmap_"
//   layer 2 (Smoothing render-method): accept iff the name starts with "smoothing_"
//   layer 0 (Material): the usage/locale/surfaceType filter chain, then the search-bar,
//            else the "is_in_use" base predicate.
// The layer-1/2 sub-views + the search-bar are NAME-based (texture names are lowercased at
// registration — Texture_GetHandle's _strlwr — so case-insensitive _strnicmp == the
// binary's raw byte compare).
//
// NOTE on the disasm vs hex-rays here: hex-rays renders LABEL_37 as `if(current_edit_layer){
// if(==1)…else if(==2)…else{usage chain}}` which wrongly implies layer 0 rejects everything.
// The disasm (0x45bc4d..0x45bc70) is the ground truth: the lightmap-prefix flag (v50, var_1D)
// and the smoothing-prefix flag (cl = the smoothing strcmp==0) are computed for EVERY material,
// and the layer-0 fallthrough (0x45bc59 not-taken) FIRST rejects lightmap_*/smoothing_* names
// (0x45bc5f cmp var_1D / 0x45bc68 test cl) BEFORE the usage chain at the SHARED loc_45BC70.
//
// LAYER 0 CHAIN (IDB 0x45bc5f..0x45bd8d, faithful to the DISASM):
//   reject if  name has "lightmap_"  prefix    (v50 != 0,  0x45bc5f cmp var_1D,al / jnz)
//   reject if  name has "smoothing_" prefix    (cl  != 0,  0x45bc68 test cl,cl / jnz)
//   reject if  tex->usage_index == 0                                          (0x45bc70)
//   reject if  usageFilter!=0       && usage_index != filter_usage_array[usageFilter].index   (0x45bc7b)
//   reject if  localeFilter!=0      && (1<<filter_locale_array[localeFilter].index & tex->tex_num_or_localefilter)==0 (0x45bc98)
//   reject if  surfaceTypeFilter!=0 && filter_surfacetype_array[surfaceTypeFilter].index != (tex->color_or_surfacetype_filter & 0x1F00000) (0x45bcbb)
//   then searchbar (prefix) if active; else accept iff tex->is_in_use         (0x45bcdd/0x45bd89)
// The usage_index/locale/surfaceFlags metadata is now populated for BOTH registration paths
// (Material_ConvertToEditorMaterial reads the raw header; Editor_AddRadiantMaterial reads the
// editor-widened Material's editorUsage/editorLocale/surfaceFlags), so the chain filters real
// data.  With no filter selected (all indices 0 — the "all" default), the chain reduces to
// "reject lightmap_/smoothing_/usage_index==0, else accept iff is_in_use" — every in-use
// material with a real usage (i.e. every browsable world material) is accepted, so the default
// view is NOT emptied.
static bool TexWnd_FilterAccept( const qtexture_s *tex )
{
    if ( !tex || !tex->name )
        return false;

    const int layer = g_qeglobals.current_edit_layer;
    if ( layer == 1 )                                          // Lightmap render-method
        return _strnicmp( tex->name, "lightmap_", 9 ) == 0;
    if ( layer == 2 )                                          // Smoothing render-method
        return _strnicmp( tex->name, "smoothing_", 10 ) == 0;

    // layer 0 (Material) — the lightmap/smoothing prefix exclusion is computed for every
    // material and rejected first (0x45bc5f / 0x45bc68), then the usage/locale/surfaceType chain.
    if ( _strnicmp( tex->name, "lightmap_", 9 ) == 0 )                          // 0x45bc5f (v50)
        return false;
    if ( _strnicmp( tex->name, "smoothing_", 10 ) == 0 )                        // 0x45bc68 (cl)
        return false;

    const unsigned char usage_index = (unsigned char)tex->usage_index;          // 0x45bc70 mov al,[esi+0Ah]
    if ( !usage_index )                                                          // 0x45bc73 test al,al; jz reject
        return false;

    const unsigned char usageFilter = texWndGlob_textureOffset.usageFilter;      // 0x45bc7b
    if ( usageFilter && usage_index != (unsigned char)filter_usage_array[usageFilter].index ) // 0x45bc8b cmp/jnz
        return false;

    const unsigned char localeFilter = texWndGlob_textureOffset.localeFilter;    // 0x45bc98
    if ( localeFilter &&
         ( ( 1 << filter_locale_array[localeFilter].index ) & tex->tex_num_or_localefilter ) == 0 ) // 0x45bca4 shl/test/jz
        return false;

    const unsigned char surfaceTypeFilter = texWndGlob_textureOffset.surfaceTypeFilter; // 0x45bcbb
    if ( surfaceTypeFilter &&
         filter_surfacetype_array[surfaceTypeFilter].index != ( tex->color_or_surfacetype_filter & 0x1F00000 ) ) // 0x45bcc4 and/cmp/jnz
        return false;

    // Search-bar (FAITHFUL, data-independent): when active, accept iff the material name
    // matches the search buffer as a case-insensitive PREFIX (IDB 0x45bcdd: _strnicmp(buffer,
    // name, strlen(buffer))).  searchbar_filter is false until a search UI is wired.
    if ( texWndGlob_textureOffset.searchbar_filter && texWndGlob_textureOffset.searchbar_buffer )
    {
        const char *q = texWndGlob_textureOffset.searchbar_buffer;
        return _strnicmp( q, tex->name, strlen( q ) ) == 0;
    }

    // base predicate (0x45bd89): show the textures the map actually references.
    return tex->is_in_use;
}

// TexWnd_IterateMaterials (IDB 0x45ba70) — advance to the next accepted material and
// compute its flow-layout slot. Returns false when the list is exhausted. The accept
// predicate is TexWnd_FilterAccept (above); the layout math below is verbatim from the IDB.
static bool TexWnd_IterateMaterials( MaterialIter_t *it )
{
    ++it->sortedIndex;
    int count = texWndGlob_textureOffset.materialCount;
    qtexture_s *tex = nullptr;
    for ( ; it->sortedIndex < count; ++it->sortedIndex )
    {
        tex = texWndGlob_textureOffset.sorted_materials[it->sortedIndex];
        it->radMtl = tex;
        if ( TexWnd_FilterAccept( tex ) )
            break;
    }
    if ( it->sortedIndex >= count )
        return false;

    // ── flow layout (verbatim from IDB 0x45ba70 tail) ──
    double w = (double)tex->width  * it->texwndscale;
    double h = (double)tex->height * it->texwndscale;
    while ( w < 8.0 || h < 8.0 )   { w *= 2.0; h *= 2.0; }   // smaller side ≥ 8
    while ( w < 32.0 && h < 32.0 ) { w *= 2.0; h *= 2.0; }   // at least one side ≥ 32
    int thumbW = Ed_RoundToInt( (float)w );
    int thumbH = Ed_RoundToInt( (float)h );
    it->thumbW = thumbW;
    it->thumbH = thumbH;

    // wrap to the next row when this thumbnail would overflow the pane width
    if ( thumbW + it->cursorX > texWndGlob_textureOffset.m_nWidth - 8 )
    {
        if ( it->rowMaxH > 0 )
        {
            it->cursorY += it->labelBand + it->rowMaxH + 4;
            it->cursorX  = TEX_MARGIN;
            it->rowMaxH  = 0;
        }
    }
    it->thumbX = it->cursorX;
    it->rowY   = it->cursorY;
    if ( it->rowMaxH < thumbH ) it->rowMaxH = thumbH;
    it->cursorX += ( thumbW < 64 ? 64 : thumbW ) + 8;
    return true;
}

BEGIN_MESSAGE_MAP( CTexWnd, CWnd )
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_MOUSEWHEEL()
    ON_WM_VSCROLL()
END_MESSAGE_MAP()

CTexWnd::CTexWnd() {}

BOOL CTexWnd::PreCreateWindow( CREATESTRUCT& cs )
{
    cs.lpszClass = AfxRegisterWndClass(
        CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor( NULL, IDC_ARROW ), NULL, NULL );
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL;
    return CWnd::PreCreateWindow( cs );
}

int CTexWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    CRect rc; GetClientRect( &rc );
    m_nWidth  = rc.Width();
    m_nHeight = rc.Height();
    texWndGlob_textureOffset.m_nWidth  = m_nWidth;
    texWndGlob_textureOffset.m_nHeight = m_nHeight;
    return 0;
}

void CTexWnd::OnSize( UINT nType, int cx, int cy )
{
    CWnd::OnSize( nType, cx, cy );
    m_nWidth = cx; m_nHeight = cy;
    texWndGlob_textureOffset.m_nWidth  = cx;
    texWndGlob_textureOffset.m_nHeight = cy;
    if ( dx.device && cx > 0 && cy > 0 )
        R_Hwnd_Resize( (HWND__ *)GetSafeHwnd(), cx, cy );
}

BOOL CTexWnd::OnEraseBkgnd( CDC* )
{
    return TRUE;   // D3D present owns the client area
}

// HitTest — pane pixel (px,py) → material index in sorted_materials, or -1. Runs the
// SAME flow iterator as DrawMaterials and finds the thumbnail whose cell contains the
// click (IDB TexWnd_SelectMaterial 0x45c520 hit-rect: x in [thumbX, thumbX+thumbW),
// y in [rowY, rowY+thumbH+labelBand]). The iterator works in UNSCROLLED coordinates,
// so the click is lifted by the scroll offset.
int CTexWnd::HitTest( int px, int py )
{
    Font_s *font = (Font_s *)g_qeglobals.d_font_list;
    if ( !font ) return -1;
    int y = py + m_scrollY;
    MaterialIter_t it;
    TexWnd_SetupIter( &it, font );
    while ( TexWnd_IterateMaterials( &it ) )
    {
        if ( px >= it.thumbX && px < it.thumbX + it.thumbW &&
             y  >= it.rowY   && y  <= it.rowY + it.thumbH + it.labelBand )
            return it.sortedIndex;
    }
    return -1;
}

// ShowMaterialStatus — the click status-bar readout (IDB TexWnd_SelectMaterial 0x45c520).
// Runs the SAME flow iterator + hit-rect as HitTest; on a hit, write "<name> W: w H: h"
// into status pane 3 (the binary's get_m_strStatus(&m_strStatus[3]) + UpdateStatusText,
// here the decoupled MainFrm_SetStatusText sink); on a miss, "Did not select a texture"
// to d_hwndStatus (faithful to the binary's SendMessageA(d_hwndStatus, ...) branch).
void CTexWnd::ShowMaterialStatus( int px, int py )
{
    Font_s *font = (Font_s *)g_qeglobals.d_font_list;
    if ( !font ) return;
    int y = py + m_scrollY;
    MaterialIter_t it;
    TexWnd_SetupIter( &it, font );
    while ( TexWnd_IterateMaterials( &it ) )
    {
        iassert( it.radMtl );
        if ( px >= it.thumbX && px < it.thumbX + it.thumbW &&
             y  >= it.rowY   && y  <= it.rowY + it.thumbH + it.labelBand )
        {
            qtexture_s *q = it.radMtl;
            MainFrm_SetStatusText( 3, va( "%s W: %i H: %i", q->name, q->width, q->height ) );
            return;
        }
    }
    ::SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                    (LPARAM)"Did not select a texture\n" );  // global, not CWnd::SendMessageA
}

// DrawMaterials — the faithful 2D THUMBNAIL grid (IDB TexWnd_DrawMaterials 0x45cc40).
// Called between ProjectionSet2D and R_EndFrame (2D pane-pixel coordinates active).
// Each visible thumbnail = R_AddCmdDraw2DImage(world material colormap) with the
// material name above it; the selected thumbnail gets an amber frame. Off-screen rows
// are laid out (for scroll extent + hit-test) but not drawn.
void CTexWnd::DrawMaterials()
{
    texWndGlob_textureOffset.m_nWidth  = m_nWidth;
    texWndGlob_textureOffset.m_nHeight = m_nHeight;

    static const float s_white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    Font_s *font = (Font_s *)g_qeglobals.d_font_list;
    if ( !font ) return;

    MaterialIter_t it;
    TexWnd_SetupIter( &it, font );
    int contentBottom = 0;

    while ( TexWnd_IterateMaterials( &it ) )
    {
        qtexture_s *q = it.radMtl;
        int rowBottom = it.rowY + it.labelBand + it.thumbH;
        if ( rowBottom > contentBottom ) contentBottom = rowBottom;

        int screenY = it.rowY - m_scrollY;                        // top of cell on screen
        if ( screenY >= m_nHeight || screenY + it.labelBand + it.thumbH < 0 )
            continue;                                             // vertical cull

        // lazily ensure the engine handle is registered (already set for map materials).
        // IDB TexWnd_DrawMaterials (0x45cc40): the registered handle must be this material's.
        if ( !q->next )
        {
            qtexture_s *rv = Texture_GetHandle( (char *)q->name );
            iassert( rv == q );
            iassert( q->next );
        }
        Material *mtl = q->next;

        float x = (float)it.thumbX;
        float yImg = (float)( it.labelBand + screenY );           // image top (label band above)
        // RB_DrawStretchPic forces TECHNIQUE_UNLIT (which samples the colormap) — always
        // present for registered editor materials (the matsys invariant, see Cam_TechAvailable).
        // Guard a partially-loaded material (NULL techniqueSet) defensively: the stretch-pic
        // backend, unlike the camera's DrawTris, has no technique-missing skip.
        if ( mtl && mtl->techniqueSet )
            R_AddCmdDraw2DImage( x, yImg, (float)it.thumbW, (float)it.thumbH,
                                 0.0f, 0.0f, 1.0f, 1.0f, s_white, mtl );

        // IDB 0x45ce1b: selected thumbnail gets a 1px-outset frame in the saved "selected
        // texture" colour colors[10], via R_DrawOutlineRect (R_AddCmd_Line2D) — not amber.
        if ( it.sortedIndex == m_selIndex )
            R_DrawOutlineRect( it.thumbX - 1, (int)yImg - 1,
                               it.thumbX + it.thumbW + 1, (int)yImg + it.thumbH + 1,
                               g_qeglobals.d_savedinfo.colors[10] );

        // IDB 0x45ce57: name label at scale 1.0 in the saved text colour colors[8].
        R_AddCmdDrawText( q->name, 0x7FFFFFFF, font, x, yImg,
                          1.0f, 1.0f, 0.0f, g_qeglobals.d_savedinfo.colors[8], 0 );
    }
    m_contentH = contentBottom + TEX_MARGIN;
}

void CTexWnd::OnPaint()
{
    CPaintDC dc( this );
    if ( !dx.device )
        return;
    HWND__ *hwnd = (HWND__ *)GetSafeHwnd();
    if ( !R_SetupRendertarget_CheckDevice( hwnd ) )
        return;

    R_BeginFrame();
    R_BeginSharedCmdList();
    // IDB CTexWnd::OnPaint @0x45db20: clear to the saved COLOR_TEXTUREBACK (colors[0]), not a
    // hardcoded dark grey.
    R_AddCmdClearScreen( 7, g_qeglobals.d_savedinfo.colors[0], 1.0f, 0 );
    // MATERIAL_COLOR for the thumbnail draw: the binary's CTexWnd::OnPaint (IDB 0x45db20) does
    // NOT set it at all (BeginFrame->AddClearCmd->SetProjection2D->R_DrawTexWnd), leaving the
    // per-command-buffer default MATERIAL_COLOR = [0,0,0,0] (gfxCmdBufInput is memset 0). The
    // .w == 0 is the load-bearing part: the world/tool UNLIT pixel shader is a vertcol_shaded*
    // variant whose ps_3_0 body is  oC0.rgb = matColor.w*(matColor.rgb - vColor*colorMap)
    // + colorMap*vColor  (c1 = MATERIAL_COLOR). With matColor.w == 0 the flat-override term
    // vanishes and the sampled colorMap shows through; with the prior kisak [1,1,1,1] (w==1) it
    // FULLY replaced every thumbnail with flat white. We DO still emit the set (vs the binary's
    // omission) because kisak's editor fog bridge rides on RB_SetMaterialColorCmd (rb_backend.cpp)
    // to seed the no-fog CONST_SRC_CODE_FOG / FOG_COLOR defaults that the vertcol_simple_fog
    // thumbnail shaders read — the editor has no per-scene fog setup. So: w == 0 (faithful
    // matColor state) + the fog bridge fires. Verified on real GPU pixels (mp_test browser,
    // GetRenderTargetData→LockRect): aa_default/_default_water 255,255,255→their real texture,
    // ac_building_* keep their texture, vs the all-white prior behaviour.
    { static const float s_matColor[4] = { 1.0f, 1.0f, 1.0f, 0.0f }; R_AddCmdSetMaterialColor( s_matColor ); }
    R_AddCmdProjectionSet2D();      // = SetProjection2D (RC_PROJECTION_SET / GFX_PROJECTION_2D)
    // IDB R_DrawTexWnd (0x45d0f0) dispatch: the layered-material sub-view when the mode
    // flag is set, else the normal thumbnail grid.  (g_texwnd_simple_layered_selection is
    // 0 until a layered-mode toggle is wired, so the grid path is the default.)
    if ( g_texwnd_simple_layered_selection == 1 )
        TexWnd_DrawLayeredMaterials();
    else
        DrawMaterials();
    UpdateScrollRange();    // IDB OnPaint 0x45db20: set the WS_VSCROLL range from the laid-out content

    R_EndFrame();
    R_IssueRenderCommands( (uint32_t)-1 );
    R_SortMaterials();
    R_CheckTargetWindow( hwnd );
}

// ApplyMaterialAtIndex — select material `idx` and apply it to the current face
// selection (the body of a thumbnail click), factored out of OnLButtonDown.
void CTexWnd::ApplyMaterialAtIndex( int idx )
{
    if ( idx < 0 || idx >= texWndGlob_textureOffset.materialCount )
        return;
    {
        m_selIndex = idx;
        qtexture_s *q = texWndGlob_textureOffset.sorted_materials[idx];
        if ( q && q->name )
        {
            // Freshen + VALIDATE the face selection before applying. Under random input a
            // prior edit can bump a brush's def->version without rebuilding its faceVis
            // (so b->version != b->def->version), or change its faceCount (reallocating
            // b->faces so selFace.face dangles) — either trips Brush_SetTexture's selFace
            // asserts. Rebuild each selected brush's faceVis (sub_477D70, no realloc when
            // faceCount is unchanged → selFace.face stays valid), then drop the whole face
            // selection if any entry is still inconsistent. (monkey hardening.)
            int n = *(int *)g_ptrSelectedFaces_GetSize;
            bool selOk = ( n >= 0 ) && ( n == 0 || selFace != nullptr );
            for ( int i = 0; selOk && i < n; ++i )
            {
                selbrush_t *b = selFace[i].brush;
                if ( !b || !b->def ) { selOk = false; break; }
                sub_477D70( b, (const float *)world_orient_matrix );
                int fi = selFace[i].index;
                if ( (unsigned)fi >= (unsigned)b->faceCount ||
                     b->version != b->def->version ||
                     selFace[i].face != &( (faceVis_s *)b->faces )[fi] )
                    selOk = false;
            }
            if ( !selOk ) { *(int *)g_ptrSelectedFaces_GetSize = 0; n = 0; }

            // IDA sub_45C0D0: a thumbnail click builds a fresh 36-byte MaterialDef from
            // the clicked qtexture, with texdef size = qtexture auto-scale * current
            // layer sampleSize. It does not preserve the previous face/current mapping.
            MaterialDef mat{};
            TexWnd_BuildClickedMaterialDef( q, &mat );         // IDB sub_45C0D0
            Texture_SetTexture( nullptr, &mat );
        }
        g_nUpdateBits = -1;          // redraw camera (the applied face) + this window
        Invalidate( FALSE );
    }
}

void CTexWnd::OnLButtonDown( UINT nFlags, CPoint point )
{
    // Status-bar readout of the clicked material (IDB OnButtonDown → TexWnd_SelectMaterial_02
    // → TexWnd_SelectMaterial), then select + apply it to the face selection.
    ShowMaterialStatus( point.x, point.y );
    int idx = HitTest( point.x, point.y );
    if ( idx >= 0 )
        ApplyMaterialAtIndex( idx );
    CWnd::OnLButtonDown( nFlags, point );
}

// IDB TexWnd_OnRightMouseContextMenu 0x45c8d0 — the texture-window right-click popup.
// VERBATIM from the binary: "View simple materials" (id 1, MF_CHECKED=8 when the view
// flag==0) / "View layered materials" (id 2, checked when ==1); in layered mode a
// MF_SEPARATOR (0x800) + "Save layered materials" (id 0x88C1).  TrackPopupMenu uses
// TPM_RETURNCMD (0x100) at the recorded screen-space down point on d_hwndTexture.
// Picking 1/2 switches g_texwnd_simple_layered_selection (+g_nUpdateBits W_TEXTURE redraw,
// drained each idle by CRadiantApp::OnIdle → UpdateWindows); 0x88C1 (35009) saves the library.
extern int  g_nUpdateBits;                // mainfrm.cpp
extern char LayeredMaterials_Save();      // layeredmaterials.cpp (IDB 0x416f40)
static void TexWnd_OnRightMouseContextMenu()
{
    HMENU PopupMenu = CreatePopupMenu();
    AppendMenuA( PopupMenu, g_texwnd_simple_layered_selection != 0 ? 0 : 8, 1u, "View simple materials" );
    AppendMenuA( PopupMenu, g_texwnd_simple_layered_selection != 1 ? 0 : 8, 2u, "View layered materials" );
    if ( g_texwnd_simple_layered_selection == 1 )
    {
        AppendMenuA( PopupMenu, 0x800u, 0, 0 );                              // MF_SEPARATOR
        AppendMenuA( PopupMenu, 0, 0x88C1u, "Save layered materials" );      // id 35009
    }
    int cmd = TrackPopupMenu( PopupMenu, 0x100u,                            // TPM_RETURNCMD
                              texWndGlob_textureOffset.m_ptDown[0],
                              texWndGlob_textureOffset.m_ptDown[1],
                              0, g_qeglobals.d_hwndTexture, 0 );
    if ( (unsigned int)(cmd - 1) > 1 )
    {
        if ( cmd == 35009 )
            LayeredMaterials_Save();                                         // IDB: LOBYTE(v1)= (return unused)
    }
    else if ( g_texwnd_simple_layered_selection != cmd - 1 )                 // cmd is 1 or 2 → 0 or 1
    {
        g_texwnd_simple_layered_selection = cmd - 1;
        // IDB writes the literal 0x10 = W_TEXTURE (disasm CMainFrame::UpdateWindows @0x427090:
        // `test bl,10h` → repaint m_pTexWnd).  qedefs.h macros were re-aligned to the binary
        // W_TEXTURE matches the binary's raw 0x10.
        g_nUpdateBits |= W_TEXTURE;
    }
}

// IDB CTexWnd::OnButtonDown 0x45c9a0 (right-button path): record the screen-space down
// point (popup anchor) + the button mask.  The right button does not select a material.
void CTexWnd::OnRButtonDown( UINT nFlags, CPoint point )
{
    POINT pt;
    GetCursorPos( &pt );
    texWndGlob_textureOffset.m_ptDown[0] = pt.x;
    texWndGlob_textureOffset.m_ptDown[1] = pt.y;
    texWndGlob_textureOffset.lastButtonDown = nFlags;
    (void)point;   // IDB OnButtonDown is standalone (records only); no base call
}

// IDB CTexWnd::OnButtonUp 0x45ca30: on right-button release, pop the view-mode context
// menu (a right-drag would set m_was_mouse_dragged and pan instead — that path is not
// yet ported, so m_was_mouse_dragged stays 0 and the click always opens the menu).
void CTexWnd::OnRButtonUp( UINT nFlags, CPoint point )
{
    if ( ( texWndGlob_textureOffset.lastButtonDown & 2 ) != 0 )      // MK_RBUTTON
    {
        if ( texWndGlob_textureOffset.m_was_mouse_dragged )
        {
            texWndGlob_textureOffset.m_was_mouse_dragged = 0;
            ShowCursor( TRUE );
        }
        else
        {
            TexWnd_OnRightMouseContextMenu();
        }
    }
    (void)nFlags; (void)point;   // IDB OnButtonUp is standalone; no base call (avoids WM_CONTEXTMENU)
}

// IDB CTexWnd::CheckScroll 0x45c7c0 — clamp `n` to [0, contentExtent - paneHeight + 16];
// on change, store it (m_scrollY == the binary's nPos_current), move the OS thumb, repaint.
// m_contentH is the binary's nPos_max (content extent, set by DrawMaterials).
int CTexWnd::CheckScroll( int n )
{
    int result = ( m_contentH - m_nHeight ) + 16;
    if ( n > result )
        n = result;
    if ( n < 0 )
        n = 0;
    if ( n != m_scrollY )
    {
        SetScrollPos( SB_VERT, n, TRUE );
        m_scrollY = n;
        Invalidate( FALSE );
        UpdateWindow();
    }
    return result;
}

// IDB sub_45C830 — set the WS_VSCROLL range/page from the content extent, then clamp.
void CTexWnd::UpdateScrollRange()
{
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof( SCROLLINFO );
    si.fMask  = SIF_RANGE | SIF_PAGE;
    si.nMin   = 0;
    si.nMax   = m_contentH + 16;          // nPos_max + 16
    si.nPage  = m_nHeight;
    SetScrollInfo( SB_VERT, &si, TRUE );
    int result = si.nMax - si.nPage;
    if ( m_scrollY > result )
    {
        m_scrollY = result;
        CheckScroll( result );
    }
}

// IDB CTexWnd::OnVScroll 0x45dc80 — the WM_VSCROLL handler.
void CTexWnd::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
    CWnd::OnVScroll( nSBCode, nPos, pScrollBar );
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof( SCROLLINFO );
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;   // 0x17
    GetScrollInfo( SB_VERT, &si, SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS );
    int n = si.nPos;
    switch ( nSBCode )
    {
        case SB_LINEUP:    CheckScroll( si.nPos - 15 );        break;
        case SB_LINEDOWN:  CheckScroll( si.nPos + 15 );        break;
        case SB_PAGEUP:    CheckScroll( si.nPos - m_nHeight ); break;
        case SB_PAGEDOWN:  CheckScroll( si.nPos + m_nHeight ); break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK: n = si.nTrackPos;
            // fall through
        default:           CheckScroll( n );                  break;
    }
}

// IDB CTexWnd::Scroll 0x45dd80 — mouse-wheel = half-page step, sign from the wheel direction.
BOOL CTexWnd::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
    int step = ( zDelta < 0 ) ? ( m_nHeight / -2 ) : ( m_nHeight / 2 );
    CheckScroll( m_scrollY - step );
    return CWnd::OnMouseWheel( nFlags, zDelta, pt );
}

// ══════════════════════════════════════════════════════════════════════════════
//  0x45B910  Material_SetMode — switch the face DISPLAY MODE (Textures→Render Method:
//  Material=0 / Lightmap=1 / Smoothing=2).  Sets g_qeglobals.current_edit_layer (the
//  MaterialDef layer index every face-texture access keys off — brush.cpp/camwnd.cpp/
//  csg.cpp/filters.cpp), invalidates the texture-window scrollbar range, radio-checks the
//  three menu items, refreshes the open surface-inspector + texture-bar, rebuilds the
//  three brush lists' display state, copies selected-face values, and requests a redraw.
//  Menu IDs 33232/33233/36100 are the binary's ID_Material/ID_Lightmap/ID_Smoothing.
// ══════════════════════════════════════════════════════════════════════════════
extern CMainFrame *g_pParentWnd;                        // engine_stubs.cpp (0x25D5A70)
extern int   g_surfwin;                                 // surfacedlg.cpp (0x23F1624)
extern void  Surf_RefreshFields();                      // surfacedlg.cpp (Select_SetTexture_2 field refresh)
extern void  SurfaceInspector_SetTexMods();             // surfacedlg.cpp (0x458270 — multi-layer snapshot)
extern void  CopySelectedFaceValues();                  // brush.cpp 0x47d130
extern void  sub_47D060( int brushList );               // brush.cpp

void Material_SetMode( int iMode )
{
    if ( iMode == g_qeglobals.current_edit_layer )
        return;

    g_qeglobals.current_edit_layer  = iMode;
    texWndGlob_textureOffset.m_bNeedRange = false;        // scrollbar range now stale

    // Radio-check the three Render Method menu items (checked iff its mode is active).
    HMENU menu = ::GetMenu( g_qeglobals.d_hwndMain );
    if ( menu )
    {
        ::CheckMenuItem( menu, 33232 /*Material*/,  iMode != 0 ? MF_UNCHECKED : MF_CHECKED );
        ::CheckMenuItem( menu, 33233 /*Lightmap*/,  iMode != 1 ? MF_UNCHECKED : MF_CHECKED );
        ::CheckMenuItem( menu, 36100 /*Smoothing*/, iMode != 2 ? MF_UNCHECKED : MF_CHECKED );
    }

    if ( g_surfwin )
    {
        SurfaceInspector_SetTexMods();                    // 0x458270 — re-snapshot the NEW layer
        Surf_RefreshFields();                             // Select_SetTexture_2 — refresh the fields
    }

    if ( g_pParentWnd && g_pParentWnd->m_wndTextureBar.m_hWnd )
        CTextureBar::GetSurfaceAttributes( &g_pParentWnd->m_wndTextureBar );

    sub_47D060( (int)(intptr_t)&active_brushes );
    sub_47D060( (int)(intptr_t)&selected_brushes );
    sub_47D060( (int)(intptr_t)&filtered_brushes );
    CopySelectedFaceValues();

    // 0x13 repaints camera, XY, and the texture browser.
    g_nUpdateBits |= ( W_CAMERA | W_XY | W_TEXTURE );     // binary 0x13
}

// ══════════════════════════════════════════════════════════════════════════════
//  0x45A520  Texture_SetMode — the Textures→Render Method RADIO (menu ids 32990..32994:
//  Wireframe / Fullbright / Normal-based Fake Lighting / View-based Fake Lighting /
//  Case textures).  Stores the picked menu id in d_savedinfo.iTextMenu and pushes the
//  matching camera draw_mode 0..4 into the 3D view, invalidating it only on a real change.
//  The default case asserts "unhandled case" at TexWnd.cpp:192 unless ASSERT_UNKOWN.
//  DISTINCT from Material_SetMode above (33232/33233/36100) — that one is the layer radio.
//  Reached via CMainFrame::OnRendermethodCaseTextures (0x4243D0, ON_COMMAND_RANGE).
// ══════════════════════════════════════════════════════════════════════════════
void Texture_SetMode( int iTexMenu )
{
    int mode;
    switch ( iTexMenu )
    {
    case 32990: mode = 0; break;      // ID_RenderMethod_Wireframe
    case 32991: mode = 1; break;      // ID_RenderMethod_Fullbright
    case 32992: mode = 2; break;      // ID_RenderMethod_Normal_basedFakeLighting
    case 32993: mode = 3; break;      // ID_RenderMethod_View_basedFakeLighting
    case 32994: mode = 4; break;      // ID_RenderMethod_Casetextures
    default:
        iassert( 0 && "unhandled case" );   // idb Assert(TexWnd.cpp, 192, 1, "unhandled case")
        return;
    }

    g_qeglobals.d_savedinfo.iTextMenu = iTexMenu;
    if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
    {
        CCamWnd *cam = g_pParentWnd->m_pCamWnd;
        if ( cam->camera.draw_mode != mode )
        {
            cam->camera.draw_mode = mode;
            g_nUpdateBits |= W_CAMERA;
        }
    }
}

// ── CTexWnd_Shutdown (0x45d1e0) — tear down the registered material set ─────────
//   The inverse of Load_Materials: free the usage/locale filter-name arrays (kisak never
//   populates these — usageCount/localeCount stay 0, so those loops are inert), then free
//   every sorted material's name (malloc'd by AllocMaterialString) + the qtexture_s itself
//   (kisak uses operator new → delete; the binary's static-CRT operator new is malloc, so
//   its j__free is equivalent), drop the editor vertex buffers (editorVB_freeBuffers), and
//   free the material-name remap list ({key,value,next} triples — null/dead in kisak until
//   the P6 remap system lands).  Called by CMainFrame::OnDestroy + QE_LoadProject (neither
//   is wired in kisak yet, so this is a ready-when-needed leaf — faithful 1:1 with the IDB).
extern void editorVB_freeBuffers();                 // r_ed_vertbuf.cpp (0x51CC00)
extern void *texWndGlob_materialNameRemap;          // engine_stubs.cpp (0x25E7A00)
struct MaterialNameRemap { char *key; char *value; MaterialNameRemap *next; };
// The usage/locale filter-name arrays (IDB filter_usage_array 0x739F80 / filter_locale_array
// 0x73A780, filter_material_t {char*name; int index} x 256) are populated by the ported
// FillTextureMenu (qe3.cpp).  usageCount/localeCount are non-zero once it runs, so these
// free loops reclaim the heap strings TexFilter_LoadMenuFile allocated for entries 1..count-1.
// (RadiantFilterEntry + the filter_usage/locale_array externs are declared up near
// TexWnd_FilterAccept, which also reads them for the layer-0 filter chain.)

void CTexWnd_Shutdown()
{
    for ( int i = 1; i < texWndGlob_textureOffset.usageCount; ++i )
        free( filter_usage_array[i].name );
    texWndGlob_textureOffset.usageCount = 0;

    for ( int j = 1; j < texWndGlob_textureOffset.localeCount; ++j )
        free( filter_locale_array[j].name );
    texWndGlob_textureOffset.localeCount = 0;

    for ( int v = 0; v < texWndGlob_textureOffset.materialCount; ++v )
    {
        qtexture_s *q = texWndGlob_textureOffset.sorted_materials[v];
        iassert( q );                                   // "texWndGlob.sortedMaterials[materialIndex]"
        free( (void *)q->name );
        iassert( q );                                   // "radMtl"
        delete q;                                       // operator new'd by Load_Materials
    }
    texWndGlob_textureOffset.materialCount = 0;
    texWndGlob_textureOffset.qtextures     = nullptr;

    editorVB_freeBuffers();

    for ( MaterialNameRemap *k = (MaterialNameRemap *)texWndGlob_materialNameRemap; k; )
    {
        MaterialNameRemap *next = k->next;
        free( k->key );
        free( k->value );
        free( k );
        texWndGlob_materialNameRemap = next;
        k = next;
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  TEXTURE-FILTER MENU SUBSYSTEM (Textures-menu Usage / Locale / Surface type submenus).
//  Ported 1:1 from the IDB:
//    TexFilter_LoadMenuFile   0x45B010 (qe3.cpp — the line-by-line table loader)
//    AppendTextureFilterMenu  0x45AFB0 (builds one CreateMenu() popup + appends it)
//    FillTextureMenu          0x45B260 (loads usage.txt/locale.txt, appends the 3 submenus)
//    TexWnd_UsageFilter       0x45B3B0  → CMainFrame::OnFilterUsage       (ON_COMMAND_RANGE 60000..60255)
//    TexWnd_localFilter       0x45B490  → CMainFrame::OnFilterLocale      (ON_COMMAND_RANGE 60256..60511)
//    TexWnd_SurfaceTypeFilter 0x45B570  → CMainFrame::OnFilterSurfaceType (ON_COMMAND_RANGE 60512..60767)
//  Wired reachable from CMainFrame::OnCreate (the QE_LoadProject analog), after the
//  menu is installed — FillTextureMenu needs GetSubMenu(GetMenu(d_hwndMain), 5) live.
// ══════════════════════════════════════════════════════════════════════════════
extern int TexFilter_LoadMenuFile( const char *txt, void *dest, int startId );  // qe3.cpp (0x45B010)
extern const char *Dvar_GetString( const char *dvarName );                       // qcommon (fs_basepath)

// filter_surfacetype_array (IDB 0x73AF80) — 29 STATIC entries {name, surfaceFlag}.  Unlike
// the usage/locale arrays (loaded from text), this is a compiled-in table; the `index` is the
// material surface-flag bit (e.g. asphalt = 22<<20), passed straight to the menu id math.
// Verbatim from the IDB .data dump (the strings are string-literals, never freed).
RadiantFilterEntry filter_surfacetype_array[29] = {
    { (char *)"all", 0x0 },
    { (char *)"asphalt", 0x1600000 },
    { (char *)"bark", 0x100000 },
    { (char *)"brick", 0x200000 },
    { (char *)"carpet", 0x300000 },
    { (char *)"ceramic", 0x1700000 },
    { (char *)"cloth", 0x400000 },
    { (char *)"concrete", 0x500000 },
    { (char *)"cushion", 0x1A00000 },
    { (char *)"dirt", 0x600000 },
    { (char *)"flesh", 0x700000 },
    { (char *)"fruit", 0x1B00000 },
    { (char *)"foliage", 0x800000 },
    { (char *)"glass", 0x900000 },
    { (char *)"grass", 0xA00000 },
    { (char *)"gravel", 0xB00000 },
    { (char *)"ice", 0xC00000 },
    { (char *)"metal", 0xD00000 },
    { (char *)"mud", 0xE00000 },
    { (char *)"painted metal", 0x1C00000 },
    { (char *)"paper", 0xF00000 },
    { (char *)"plaster", 0x1000000 },
    { (char *)"plastic", 0x1800000 },
    { (char *)"rock", 0x1100000 },
    { (char *)"rubber", 0x1900000 },
    { (char *)"sand", 0x1200000 },
    { (char *)"snow", 0x1300000 },
    { (char *)"water", 0x1400000 },
    { (char *)"wood", 0x1500000 },
};

// AppendTextureFilterMenu (0x45AFB0) — build ONE submenu popup ("Usage"/"Locale"/"Surface type")
// from a filter array and append it to the parent Textures menu.  Each non-NULL entry → a menu
// item with id (idBase + arrayIndex); each NULL entry (a "<separator>" loaded by the loader) →
// MF_SEPARATOR.  VERBATIM from the IDB (a3=array base, a4=count, a5=idBase).
static BOOL AppendTextureFilterMenu( HMENU hMenu, LPCSTR lpNewItem,
                                     const RadiantFilterEntry *array, int count, int idBase )
{
    HMENU sub = CreateMenu();
    for ( int v5 = 0; v5 < count; ++v5 )
    {
        if ( array[v5].name )                                   // 0x45afd3
            AppendMenuA( sub, MF_STRING, v5 + idBase, array[v5].name ); // 0x45afee
        else
            AppendMenuA( sub, MF_SEPARATOR, 0, 0 );             // 0x45afe1 (flag 0x800 = MF_SEPARATOR)
    }
    return AppendMenuA( hMenu, MF_POPUP, (UINT_PTR)sub, lpNewItem ); // 0x45b005 (flag 0x10 = MF_POPUP)
}

// Resolve the binary's "../deffiles/materials/<x>.txt" path.  The editor runs from the install
// bin\, where "../deffiles/.." is the deffiles tree; this port can launch from anywhere, so try
// the faithful relative path first, then <fs_basepath>\deffiles\materials\<x>.txt.  Returns the
// resolved path in `out` (the caller passes it straight to TexFilter_LoadMenuFile/fopen).
static const char *ResolveDeffilePath( const char *relative, char *out, size_t outSize )
{
    // 1) faithful relative path (cwd == install bin\).
    FILE *probe = fopen( relative, "rb" );
    if ( probe ) { fclose( probe ); _snprintf( out, outSize - 1, "%s", relative ); out[outSize - 1] = 0; return out; }
    // 2) <fs_basepath>\<relative-without-leading-../>  (the known install layout).
    const char *base = Dvar_GetString( "fs_basepath" );
    const char *tail = relative;
    while ( *tail == '.' || *tail == '/' || *tail == '\\' ) ++tail;   // strip leading "../"
    if ( base && base[0] )
    {
        _snprintf( out, outSize - 1, "%s\\%s", base, tail );
        out[outSize - 1] = 0;
        return out;
    }
    _snprintf( out, outSize - 1, "%s", relative ); out[outSize - 1] = 0;
    return out;                                                 // fall back to the relative path
}

// FillTextureMenu (0x45B260) — populate the Textures menu's Usage/Locale/Surface-type filter
// submenus.  On first call (both counts 0) it loads usage.txt → filter_usage_array (ids from
// 60000) and locale.txt → filter_locale_array (ids from 60256), then appends all three submenus
// and seeds the decal locale bit (texWndGlob.unk_8 = 1<<localeIndex("decal")) used by filters.cpp
// case-6 "Misc decal".  Finally radio-checks the active usage/locale/surfaceType item ("all" = 0).
// VERBATIM from the IDB; the only deviation is the path resolution (see ResolveDeffilePath).
DWORD FillTextureMenu()
{
    HMENU menu    = GetMenu( g_qeglobals.d_hwndMain );          // 0x45b26b
    HMENU subMenu = GetSubMenu( menu, 5 );                      // 0x45b27f (5 = the Textures top-level menu)

    if ( !texWndGlob_textureOffset.usageCount && !texWndGlob_textureOffset.localeCount ) // 0x45b28a
    {
        char path[1024];
        texWndGlob_textureOffset.usageCount =
            TexFilter_LoadMenuFile( ResolveDeffilePath( "../deffiles/materials/usage.txt", path, sizeof( path ) ),
                                    filter_usage_array, 1 );    // 0x45b2a9 (startId 1)
        texWndGlob_textureOffset.localeCount =
            TexFilter_LoadMenuFile( ResolveDeffilePath( "../deffiles/materials/locale.txt", path, sizeof( path ) ),
                                    filter_locale_array, 0 );   // 0x45b2b6 (startId 0)
    }

    AppendTextureFilterMenu( subMenu, "Usage",        filter_usage_array,       texWndGlob_textureOffset.usageCount,  60000 ); // 0x45b2d2
    AppendTextureFilterMenu( subMenu, "Locale",       filter_locale_array,      texWndGlob_textureOffset.localeCount, 60256 ); // 0x45b2f1
    AppendTextureFilterMenu( subMenu, "Surface type", filter_surfacetype_array, 29,                                   60512 ); // 0x45b30b

    texWndGlob_textureOffset.usageFilter       = 0;            // 0x45b31b
    texWndGlob_textureOffset.localeFilter      = 0;            // 0x45b322
    texWndGlob_textureOffset.surfaceTypeFilter = 0;            // 0x45b330
    for ( int v2 = 0; v2 < texWndGlob_textureOffset.localeCount; ++v2 ) // 0x45b330..0x45b380
    {
        const char *name = filter_locale_array[v2].name;       // 0x45b332
        if ( name && !_stricmp( name, "decal" ) )              // 0x45b343
            texWndGlob_textureOffset.unk_8 = 1 << filter_locale_array[v2].index; // 0x45b35d
    }

    CheckMenuItem( subMenu, texWndGlob_textureOffset.usageFilter       + 60000, MF_CHECKED ); // 0x45b384 (flag 8 = MF_CHECKED)
    CheckMenuItem( subMenu, texWndGlob_textureOffset.localeFilter      + 60256, MF_CHECKED ); // 0x45b397
    return CheckMenuItem( subMenu, texWndGlob_textureOffset.surfaceTypeFilter + 60512, MF_CHECKED ); // 0x45b3ac
}

// ── Menu-item handlers (ON_COMMAND_RANGE) ──────────────────────────────────────
// All three: mark every registered material in-use (the "Showing all textures" reset — the
// actual per-filter predicate is applied by TexWnd_FilterAccept on the next paint), then radio-
// check the chosen submenu item and store the filter index.  Verbatim from the IDB; the list is
// walked head→tail via qtexture_s::prev (texWndGlob.qtextures is the list head linked by ->prev).
static void TexWnd_ShowAllTextures()
{
    for ( qtexture_s *q = texWndGlob_textureOffset.qtextures; q; q = q->prev ) // 0x45b3b0/0x45b3c4 (qtextures[9] = +0x24 = prev)
        q->is_in_use = 1;                                       // 0x45b3c0 (+8)
    Sys_Printf( "Showing all textures...\n" );                  // 0x45b3d0
    g_nUpdateBits |= W_TEXTURE;                                 // 0x45b3d5 (or g_nUpdateBits, 10h — binary W_TEXTURE==0x10)
}

void TexWnd_UsageFilter( int index )                           // 0x45B3B0
{
    TexWnd_ShowAllTextures();
    iassert( index >= 0 && index < texWndGlob_textureOffset.usageCount ); // 0x45b3e9 TexWnd.cpp:618
    if ( index != texWndGlob_textureOffset.usageFilter )       // 0x45b412
    {
        HMENU subMenu = GetSubMenu( GetMenu( g_qeglobals.d_hwndMain ), 5 ); // 0x45b41f/0x45b441
        CheckMenuItem( subMenu, texWndGlob_textureOffset.usageFilter + 60000, MF_UNCHECKED ); // 0x45b445
        CheckMenuItem( subMenu, index + 60000, MF_CHECKED );   // 0x45b451
        texWndGlob_textureOffset.usageFilter = (unsigned char)index; // 0x45b459 (mov ...usageFilter, bl)
        g_nUpdateBits |= W_TEXTURE;                            // 0x45b480 (or g_nUpdateBits, 10h — binary W_TEXTURE==0x10)
    }
}

void TexWnd_localFilter( int index )                           // 0x45B490
{
    TexWnd_ShowAllTextures();
    iassert( index >= 0 && index < texWndGlob_textureOffset.localeCount ); // 0x45b4c9 TexWnd.cpp:638
    if ( index != texWndGlob_textureOffset.localeFilter )      // 0x45b4f2
    {
        HMENU subMenu = GetSubMenu( GetMenu( g_qeglobals.d_hwndMain ), 5 ); // 0x45b4ff/0x45b521
        CheckMenuItem( subMenu, texWndGlob_textureOffset.localeFilter + 60256, MF_UNCHECKED ); // 0x45b525
        CheckMenuItem( subMenu, index + 60256, MF_CHECKED );   // 0x45b531
        texWndGlob_textureOffset.localeFilter = (unsigned char)index; // 0x45b539 (mov ...localeFilter, bl)
        g_nUpdateBits |= W_TEXTURE;                            // 0x45b560 (or g_nUpdateBits, 10h — binary W_TEXTURE==0x10)
    }
}

void TexWnd_SurfaceTypeFilter( unsigned int index )            // 0x45B570
{
    TexWnd_ShowAllTextures();
    iassert( index <= 0x1C );                                  // 0x45b5a1 TexWnd.cpp:658 (index < 29)
    if ( index != (unsigned int)texWndGlob_textureOffset.surfaceTypeFilter ) // 0x45b5cf
    {
        HMENU subMenu = GetSubMenu( GetMenu( g_qeglobals.d_hwndMain ), 5 ); // 0x45b5dc/0x45b5fe
        CheckMenuItem( subMenu, texWndGlob_textureOffset.surfaceTypeFilter + 60512, MF_UNCHECKED ); // 0x45b602
        CheckMenuItem( subMenu, index + 60512, MF_CHECKED );   // 0x45b60e
        texWndGlob_textureOffset.surfaceTypeFilter = (unsigned char)index; // 0x45b616 (mov ...surfaceTypeFilter, bl)
        g_nUpdateBits |= W_TEXTURE;                            // 0x45b63d (or g_nUpdateBits, 10h — binary W_TEXTURE==0x10)
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  LAYERED-MATERIAL SUB-VIEW (R_DrawTexWnd's g_texwnd_simple_layered_selection==1 branch).
//  Draws the loaded layered-material library (lyrMtlGlob_Layers[], from layeredmaterials.cpp)
//  as a vertical list of outlined thumbnails: per entry a name label, then each layer's
//  texture fit into a 64×64 cell, framed by a 2D outline rect.  Ported from the IDB:
//    R_DrawOutlineRect            0x45cb10
//    TexWnd_DrawLayeredMaterialEntry  0x45cea0 (sub_45CEA0)
//    TexWnd_DrawLayeredMaterials  0x45d080
//  The 2D line command R_AddCmd_Line2D (0x4fd180) is the new gfx_d3d/r_rendercmds.cpp leaf.
// ══════════════════════════════════════════════════════════════════════════════
extern char  Byte4PackPixelColor( float *from, GfxColor *out );  // engine_stubs.cpp (IDB 0x402ac0)
extern uint8_t lyrMtlGlob_Layers[];                              // layeredmaterials.cpp (0x1814D00)
extern int   lyrMtlGlob_entryCount;                              // layeredmaterials.cpp (0x1814CFC)
extern int   lyrMtlWndGlob_active_layer_material;                // layeredmaterialwnd.cpp (0x181F510)

// The texture-window MODE flag (IDB g_texwnd_simple_layered_selection 0x25E79FC).  0 = the
// normal thumbnail grid (DrawMaterials), 1 = this layered-material sub-view.  Read by the
// whole texwnd cluster in the binary (OnPaint/scroll/right-click/select); kisak does not yet
// wire a toggle, so it stays 0 and the grid path runs unchanged.  Defined here (the file that
// owns the texture-window draw dispatch).
int g_texwnd_simple_layered_selection = 0;

// 84-byte library entry view (mirrors layeredmaterials.cpp's offset enum + the authoring
// window's LyrMtlEntry).  Per the IDB cap a library entry holds at most ONE layer, but the
// draw indexes layer i at handle offset 0x50 + 8*i (faithful to sub_45CEA0's a1[2*i+20]).
struct LyrMtlDrawEntry
{
    char        name[64];       // 0x00  the entry name (passed straight to R_AddCmdDrawText)
    int         nextId;         // 0x40
    int         layerCount;     // 0x44  a1[17]
    int         _pad48;         // 0x48
    int         layerId0;       // 0x4C  layer[0].id
    qtexture_s *layerHandle0;   // 0x50  layer[0].handle (radMtl)  — a1[20]
};
static_assert( sizeof( LyrMtlDrawEntry ) == 84, "LyrMtlDrawEntry must match the 84-byte library stride" );
// layer i's handle pointer lives at 0x50 + 8*i (the {id,handle} pair stride is 8).
static inline qtexture_s *LyrMtl_LayerHandle( const LyrMtlDrawEntry *e, int i )
{
    return *(qtexture_s *const *)( (const char *)e + 0x50 + 8 * i );
}

// R_DrawOutlineRect (IDB 0x45cb10) — draw a 2D rectangle outline (xL,yT)-(xR,yB) in
// colour `rgba` via one R_AddCmd_Line2D(4 lines).  The binary builds 8 GfxPointVertex
// (4 edges, z=0, every vertex carrying the same packed colour); transcribed here as a
// clean edge loop (behaviourally identical — same 8 vertices, same order: top, right,
// bottom, left).
static void R_DrawOutlineRect( int xL, int yT, int xR, int yB, const float *rgba )
{
    GfxColor col;
    Byte4PackPixelColor( const_cast<float *>( rgba ), &col );

    // 4 line segments (2 verts each) = the rectangle perimeter, matching the IDB vertex
    // order: 0:(xL,yT)->(xR,yT) top, 1:(xR,yT)->(xR,yB) right, 2:(xR,yB)->(xL,yB) bottom,
    // 3:(xL,yB)->(xL,yT) left.
    const int ex[8] = { xL, xR,  xR, xR,  xR, xL,  xL, xL };
    const int ey[8] = { yT, yT,  yT, yB,  yB, yB,  yB, yT };

    GfxPointVertex verts[8];
    for ( int i = 0; i < 8; ++i )
    {
        verts[i].xyz[0]  = (float)ex[i];
        verts[i].xyz[1]  = (float)ey[i];
        verts[i].xyz[2]  = 0.0f;
        *(unsigned int *)verts[i].color = col.packed;
    }
    R_AddCmd_Line2D( 4, 1, verts );
}

// TexWnd_DrawLayeredMaterialEntry (IDB sub_45CEA0) — draw ONE layered-material entry at
// the running Y cursor `*py`: a name label, each layer's texture (aspect-fit into a 64×64
// cell), and an outline rect framing the row (highlighted when this is the active entry).
// Advances *py by the row height.  `entry` is the 84-byte LyrMtlDrawEntry; its first 64
// bytes are the name string the binary passes directly to R_AddCmdDrawText.
static void TexWnd_DrawLayeredMaterialEntry( LyrMtlDrawEntry *entry, int *py )
{
    Font_s *font = (Font_s *)g_qeglobals.d_font_list;
    int     cursorX  = 8;                                 // v25 — per-layer x cursor
    int     fontH    = font ? font->pixelHeight : 0;      // sub_5120A0(d_font_list)

    int oldY = *py;
    *py += fontH;
    // label visible band check (IDB: !(fontH+oldY < 0) && *py - fontH < m_nHeight)
    if ( !( fontH + oldY < 0 ) && *py - fontH < texWndGlob_textureOffset.m_nHeight )
        R_AddCmdDrawText( entry->name, 0x7FFFFFFF, font, 8.0f, (float)*py,
                          1.0f, 1.0f, 0.0f, g_qeglobals.d_savedinfo.colors[8], 0 );

    // thumbnail-row vertical cull
    if ( *py + 64 < 0 || *py >= texWndGlob_textureOffset.m_nHeight )
    {
        *py += 72;
        return;
    }

    int frameRight = cursorX;                             // v2 — final cursorX after layers
    int layerLast  = entry->layerCount - 1;               // v9 = a1[17] - 1
    if ( layerLast >= 0 )
    {
        // Walk layers HIGH->LOW (the IDB iterates v26 = &a1[2*v9+20] downward by 2 dwords).
        for ( int i = layerLast; i >= 0; --i )
        {
            qtexture_s *handle = LyrMtl_LayerHandle( entry, i );

            // KISAK: an unrealized layer has no drawable technique; keep its layout slot.
            // In the live GUI the layer handle (Texture_GetHandle
            // at parse) and its render Material are always realized; off the realize path
            // (headless / pre-first-realize) handle->next can be NULL and RB_DrawStretchPic
            // has no technique-missing skip.  Skip the image when unrealized — geometry/
            // layout (the cursor advance + outline) stays identical.  (Behaviour-identical
            // in the GUI; matches DrawMaterials' own mtl/techniqueSet guard.)
            int w = handle ? handle->width  : 64;
            int h = handle ? handle->height : 64;

            int fitW = w, fitH = h;                       // v12/v27, v11/v28
            if ( w >= h )                                 // landscape
            {
                if ( w > 64 ) { fitW = 64; fitH = ( h << 6 ) / w; }   // (h*64)/w, truncated
            }
            else                                          // portrait
            {
                if ( h > 64 ) { fitW = ( w << 6 ) / h; fitH = 64; }   // (w*64)/h, truncated
            }

            float ix = (float)( cursorX + ( 64 - fitW ) / 2 );        // v15 (centered in cell)
            float iy = (float)( *py    + ( 64 - fitH ) / 2 );         // v16
            if ( handle && handle->next && handle->next->techniqueSet )
                R_AddCmdDraw2DImage( ix, iy, (float)fitW, (float)fitH,
                                     0.0f, 0.0f, 1.0f, 1.0f, colorWhite, handle->next );

            cursorX += 72;                                // v25 += 72
        }
        frameRight = cursorX;                             // v2 = v25
    }

    // outline colour: active entry = colors[10] (highlight), else colors[8].
    const float *frameCol =
        ( (int)(intptr_t)entry == lyrMtlWndGlob_active_layer_material )
            ? g_qeglobals.d_savedinfo.colors[10]
            : g_qeglobals.d_savedinfo.colors[8];
    if ( entry->layerCount )
        frameRight -= 8;                                  // trim the trailing +72 gap to +64
    R_DrawOutlineRect( 7, *py - 1, frameRight + 1, *py + 65, frameCol );
    *py += 72;
}

// TexWnd_DrawLayeredMaterials (IDB 0x45d080) — the layered-material sub-view: lay the
// library entries out top-to-bottom (scrolled by nPos[layer].nPos_layered_current),
// drawing each via TexWnd_DrawLayeredMaterialEntry, and record the laid-out content
// extent into nPos[layer].nPos_layered_max.  Returns the content height.
static int TexWnd_DrawLayeredMaterials()
{
    int layer = g_qeglobals.current_edit_layer;
    int y     = 8 - texWndGlob_textureOffset.nPos[layer].nPos_layered_current;

    if ( lyrMtlGlob_entryCount > 0 )
    {
        LyrMtlDrawEntry *e = (LyrMtlDrawEntry *)lyrMtlGlob_Layers;
        for ( int i = 0; i < lyrMtlGlob_entryCount; ++i, ++e )
            TexWnd_DrawLayeredMaterialEntry( e, &y );
        layer = g_qeglobals.current_edit_layer;           // (IDB re-reads after the loop)
    }

    texWndGlob_textureOffset.nPos[layer].nPos_layered_max =
        y + texWndGlob_textureOffset.nPos[layer].nPos_layered_current;
    return y;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Texture_SetTexture (IDB 0x45be50) — "make a MaterialDef the current editor
//  texture".  Copies it into random_texture_stuff[layer], realizes its handles,
//  optionally stores a patch-vertex texcoord projection, APPLIES it to the
//  selection (Brush_SetTexture), updates the find/layered-material windows, and
//  scrolls the browser to it (CheckScroll).  Driven by Drag_Begin's middle-button
//  texture pick and CreateEntityFromName's trigger/volume default material.
// ═════════════════════════════════════════════════════════════════════════════

// TexWnd_02 (IDB 0x45b690) — the MaterialDef_02 realize callback int(*)(qtexture_s*).
// Lazily registers the radMtl's engine handle.  radMtl->next (qtexture_s+0) is the
// rendered Material*; radMtl->name (qtexture_s+4) is the material name.
static int TexWnd_02( qtexture_s *radMtl )
{
    iassert( radMtl );                                     // 0x45b6a7 (TexWnd.cpp:688, L0 "radMtl")
    if ( !radMtl->next )
    {
        RadiantMaterialInfo localRaw;                      // IDB v1[56] equivalent for qtexture metadata
        radMtl->next = Register_WorldMaterial( radMtl->name, &localRaw );
    }
    return (int)(intptr_t)radMtl->next;                   // callback return; MaterialDef_02 discards it
}

// sub_415CE0 / sub_415D40 (IDB 0x415ce0 / 0x415d40).
// These push the picked material name into the CFindTextureDlg Find (sub_415CE0,
// str_set @0x78) / Replace (sub_415D40, str_set @0x7C) field, selected by byte_73C380
// (which edit last had focus; toggled by the EN_SETFOCUS handlers sub_415DF0/sub_415E00).
// The IDB body is UpdateData(TRUE) → if(control populated @0x88) str_set(member) →
// UpdateData(FALSE); the kisak dialog stores both fields directly in its EDIT controls,
// so that collapses to a ::SetWindowTextA — implemented as CFindTextureDlg::SetFindText /
// SetReplaceText.  The find-dialog-visible branch below now runs the real calls.

// Texture_SetTexture (IDB 0x45be50).  a1 = patch-vertex projection block (or null);
// a2 = the MaterialDef* to make current.  Returns the CheckScroll result / 0.
//   random_texture_stuff stride is 0x834 (2100) bytes/layer; the
//   patch-projection copy uses raw byte arithmetic (transcribed verbatim from disasm
//   0x45bee9-0x45bf3e) over an opaque grid (width@0, height@4, control points at +0x44,
//   10 floats/point = 40B, 160 floats/row = 0x280B; per-layer ST pair selected by +2*layer).
char Texture_SetTexture( const int *a1, MaterialDef *a2 )
{
    const int layer = g_qeglobals.current_edit_layer;
    curTexWndLayer_t *cur = &g_qeglobals.random_texture_stuff[layer];

    // 1) Stamp the MaterialDef into the per-layer current-texture template (36 bytes).
    memcpy( cur, a2, 36u );                                // 0x45be73 rep movsd (ecx=9)

    // 2) Realize its handles (radMtl->cb once / lyrMtl->cb per layer).
    MaterialDef_02( &cur->mtl, TexWnd_02 );                // 0x45be8e

    // base for the patch-projection block writes (cur + 0x28 onward, byte-addressed).
    char *curBytes = (char *)cur;

    // 3) Optional patch-vertex texcoord projection (a1 != 0).
    if ( a1 )
    {
        *(int *)( curBytes + 40 ) = 1;                     // 0x45bea9  has-projection flag
        *(int *)( curBytes + 44 ) = a1[0];                // 0x45bebe  grid width
        *(int *)( curBytes + 48 ) = a1[1];                // 0x45bed4  grid height
        int v3 = 0, v18 = 0;
        if ( a1[0] > 0 )                                  // 0x45beda
        {
            int v5 = a1[1];
            int v6 = 0, v17 = 0;
            do                                            // outer: rows (v3 < a1[0])
            {
                int v7 = 0;
                if ( v5 > 0 )
                {
                    int v8 = v3 << 7;                     // 0x45beef  row dest byte offset (v3*128)
                    do                                    // inner: columns (v7 < a1[1])
                    {
                        float *dst = (float *)( curBytes + 52 + v8 );          // 0x45bf02  cur+0x34+v8
                        const float *src = (const float *)&a1[2 * v6 + 17 + 2 * layer]; // 0x45bf09
                        ++v7;
                        v8 += 8;
                        v6 += 10;
                        dst[0] = src[0];                  // 0x45bf18  ST pair
                        dst[1] = src[1];                  // 0x45bf1d
                        v5 = a1[1];
                    } while ( v7 < v5 );
                    v3 = v18;
                }
                ++v3;
                v6 = v17 + 160;                           // 0x45bf30  next row src base
                v18 = v3;
                v17 += 160;
            } while ( v3 < a1[0] );
        }
    }
    else
    {
        *(int *)( curBytes + 40 ) = 0;                    // 0x45bf4e  no projection
    }

    g_nUpdateBits |= W_TEXTURE;                            // 0x45bf55  W_TEXTURE redraw (bit 0x10)

    // 4) Assert a valid MtlDef (exactly one of lyrMtl/radMtl set) — verbose, cross-file.
    if ( ( ( *(unsigned *)a2 != 0 ) + ( *( (unsigned *)a2 + 1 ) != 0 ) ) != 1 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                85, 0, "%s", "MtlDef_IsValid( mtlDef )" );

    // material name = shaderName (lyrMtl) if set, else radMtl->name.
    const char *name;
    if ( a2->lyrMtl )
        name = *(const char **)a2;                        // 0x45bf9b  (lyrMtl as char* — IDB reads a2[0])
    else
        name = *(const char **)( *( (int *)a2 + 1 ) + 4 );// 0x45bfa8  radMtl->name (a2->radMtl + 4)

    // 5) Find-dialog: push `name` into the field that last had focus (0x45bfba-0x45bfe5).
    //    IDB: byte_73C380 ? sub_415CE0(name) : sub_415D40(name) — fill Find vs Replace.
    if ( g_dlgFind && g_dlgFind->GetSafeHwnd() && ::IsWindowVisible( g_dlgFind->GetSafeHwnd() ) )
    {
        if ( byte_73C380 )
            g_dlgFind->SetFindText( name );      // 0x45bfd3  sub_415CE0
        else
            g_dlgFind->SetReplaceText( name );   // 0x45bfda  sub_415D40
    }

    // 6) Either add the radMtl as a live layered-material layer, or APPLY to the selection.
    if ( texWndGlob_textureOffset.unk_bool )              // layered-material "live add" armed
    {
        LayeredMaterialWnd_RadMtl( a2->radMtl );          // 0x45bfeb
    }
    else if ( !g_dlgFind || !g_dlgFind->GetSafeHwnd()
              || !::IsWindowVisible( g_dlgFind->GetSafeHwnd() ) )
    {
        Brush_SetTexture( a2, 1 );                        // 0x45c00c  THE APPLY
    }

    // 7) Iterate the browser to the chosen material and scroll it into view.
    MaterialIter_t iter;
    TexWnd_SetupIter( &iter, (Font_s *)g_qeglobals.d_font_list );  // 0x45c01e
    if ( !TexWnd_IterateMaterials( &iter ) )              // 0x45c028
        return 0;
    while ( 1 )
    {
        iassert( iter.radMtl );                           // 0x45c051 (TexWnd.cpp:1027, L0 "iter.radMtl")
        if ( !_stricmp( name, iter.radMtl->name ) )       // 0x45c061
            break;
        if ( !TexWnd_IterateMaterials( &iter ) )          // 0x45c070
            return 0;
    }

    // scroll target (verbatim 0x45c080-0x45c0bd): keep the matched thumbnail visible.
    int nPos = texWndGlob_textureOffset.nPos[layer].nPos_current;
    if ( iter.labelBand + iter.thumbH + iter.rowY + 8 > nPos + texWndGlob_textureOffset.m_nHeight )
        nPos = iter.thumbH + iter.labelBand - texWndGlob_textureOffset.m_nHeight + iter.rowY + 8;
    if ( iter.rowY < nPos )
        nPos = iter.rowY;
    // IDB: CTexWnd::CheckScroll(nPos) — __usercall taking n@<eax>, operates entirely on globals
    // (texWndGlob/d_hwndTexture); its `this` register is dead.  The kisak port made it a CWnd
    // member, so route through the texture-window instance (NULL headless → nothing to scroll).
    if ( g_pParentWnd && g_pParentWnd->m_pTexWnd )
        return (char)g_pParentWnd->m_pTexWnd->CheckScroll( nPos );  // 0x45c0bd
    return 0;
}

// 0x45B650 — Texture_ResetPosition: scroll the texture browser back to the top and force a
// re-layout.  The binary walks texWndGlob.nPos[0..2] zeroing each nPos_current (the per-layer
// scroll offset), then (simple mode: g_texwnd_simple_layered_selection==0) calls
// TexWnd_SelectMaterial_02(9,9).  That helper reaches TexWnd_SelectMaterial, which calls
// sub_45C0D0 -> Texture_SetTexture for the thumbnail under (9,9), making the first visible
// material the current brush texture.
// Called by CMainFrame::CheckTextureScale (texture-window-scale change): after the thumbnails
// re-lay-out at the new scale the old scroll offset is meaningless, so reset it.
//
// PORT NOTE: this port keeps the live scroll offset on CTexWnd::m_scrollY (the binary's
// nPos_current lives in the global nPos[]).  So the faithful port zeros the global nPos[]
// (data-faithful) AND resets the live window's m_scrollY to 0, then runs the same hit-test/apply
// path at (9,9) before repainting.  Headless (no m_pTexWnd) -> the global reset alone.
void Texture_ResetPosition()
{
    for ( int layer = 0; layer < 3; ++layer )
        texWndGlob_textureOffset.nPos[layer].nPos_current = 0;   // 0x45b660 loop (nPos[0..2])

    if ( g_pParentWnd && g_pParentWnd->m_pTexWnd )
    {
        CTexWnd *tex = g_pParentWnd->m_pTexWnd;
        tex->m_scrollY = 0;
        if ( g_texwnd_simple_layered_selection == 0 )             // 0x45b677
        {
            int hit = tex->HitTest( 9, 9 );                       // TexWnd_SelectMaterial_02(9,9)
            if ( hit >= 0 )
            {
                tex->ShowMaterialStatus( 9, 9 );
                tex->ApplyMaterialAtIndex( hit );                 // sub_45C0D0 -> Texture_SetTexture
            }
        }
        tex->CheckScroll( 0 );
    }
}
