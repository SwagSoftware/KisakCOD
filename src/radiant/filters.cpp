#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// CoD4Radiant filter system parsed from RadiantFilters.txt; each filter entry
// is a linked list node (filter_entry_s) in one of four category lists hanging off
// g_qeglobals.d_filterGlobals_*.

#include "stdafx.h"
#include "qe3.h"
#include <universal/q_parse.h>   // Com_Parse / Com_ParseOnLine — the filter file grammar
#include <map>                    // faceTexMap (IDB std::map<std::string,int>)
#include <string>

// ── forward declarations of helpers from materialdef.cpp / engine_stubs.cpp ──
struct qtexture_s;
extern void MaterialDef_02(MaterialDef *def, int (*cb)(qtexture_s *));  // 0x431520
extern int  MaterialDef_05(qtexture_s *mtl);                  // 0x431840 — tex_num_or_localefilter (case 6 decal)
extern int  MaterialDef_07(qtexture_s *mtl);                  // 0x431910 — callback for MaterialDef_02
extern int  MaterialDef_09(qtexture_s *mtl);                  // 0x431a00 — color_or_surfacetype_filter (case 7)
struct LayerMaterialDef;
extern LayerMaterialDef *Materialdef_GetName(MaterialDef *m); // 0x431640 — radMtl->name / (char*)lyrMtl (case 3)
extern bool Materialdef_Realize(MaterialDef *m);
extern int  dword_181F51C;                                    // materialdef realize-state flag
extern int  TexWnd_GetDecalLocaleBit();                       // texwnd.cpp — texWndGlob.unk_8 (case 6 decal seed)
extern entity_s *world_entity;                                // map.cpp — the worldspawn entity (case 6 `world`)
extern void Assert(const char *file, int line, int type, const char *fmt, ...); // 0x49cea0
extern bool Entity_HasEpairMatch( entity_s *e, const char *key, const char *val ); // entity.cpp 0x483930
extern char *AllocMaterialString( const char *s );           // 0x48B030 (string heap-copy)
extern int  Sys_Printf( const char *fmt, ... );              // win_qe3.cpp 0x499E90 (console print)
extern int  g_nUpdateBits;                                   // engine_stubs.cpp / qe3 (view-invalidation bits)
extern "C" int _stricmp( const char *, const char * );
extern "C" char *_strlwr( char * );
extern "C" void free( void * );
// (strcmp/strncpy/strlen come from <cstring> via stdafx.h)

// prefs.cpp — for the user filter path (g_PrefsDlg->m_strUserFilterPath).
#include "prefs.h"

// ═════════════════════════════════════════════════════════════════════════════
//  CONDITION-TREE PARSER (DynamicFilter_ParseCondition + leaf parsers + qe3_cpp_01)
//
//  The filter file (raw\..\RadiantFilters.txt) defines, per filter entry, a boolean
//  condition over leaf predicates:
//      Texture [All] [*]<name>     contents node {name@0, flags@4: bit0=`*`, bit1=All}  case 3
//      Contents [All] <name>       {contentBits@0, flags@4: bit1=All}                   case 4
//      Misc <world|terrain|curve|decal>  {name@0 (lowercased)}                          case 6
//      KeyValue <key> <value>[*]   {key@0, value@4, flags@8: bit0=`*` wildcard}         case 5
//      Surface [All] <name>        {surfaceBits@0, flags@4: bit1=All}                    case 7
//      EClassFlag <name>           {eclassBits@0}                                        case 8
//  combined with `&&` (surfaceFlags=1, AND), `||` (=2, OR), `!` (negate flag@+4) and
//  parentheses.  The evaluator is FilterCondition_Eval (sub_412170) below; the case
//  numbers here are EXACTLY the surfaceFlags values it switches on.
//
// Each filter_info_s node is allocated as 16 bytes (operator new(0x10)).
//  even though the named-fields struct is 12 — the 4th DWORD at +12 is the AND/OR
//  "next sibling" link, read by FilterCondition_Eval as `info[1].surfaceFlags` (byte
//  +12) and chased by Filters_FreeCondTree as `info[3]`.  We allocate 16 and write
//  +12 by hand (same pattern as Layers_BuildFilter).  Leaf contents nodes are 8 bytes
//  ({bits/name@0, flags@4}) except KeyValue which is 12 ({key,value,flags}).
//
//  PARSE IDIOM: the binary inlined Com_Parse/Com_ParseOnLine, so the IDA shows the
//  ungetToken-restore prologue + Com_ParseExt(.,1)/(.,0) open-coded.  Per the
//  established radiant idiom (entity.cpp): prologue + Com_ParseExt(.,1) == Com_Parse;
//  prologue + Com_ParseExt(.,0) (== _GetToken @0x4B83E0) == Com_ParseOnLine.  We call
//  the engine functions directly — there is no `g_parse` global in the kisak engine
//  (it uses Com_GetParseThreadInfo); these calls are the faithful equivalent.
// ═════════════════════════════════════════════════════════════════════════════

// The condition-type case values (filter_info_s.surfaceFlags) — match sub_412170.
enum {
    FILT_AND       = 1,   // children all-match
    FILT_OR        = 2,   // children any-match
    FILT_TEXTURE   = 3,   // material-name match           (sub_411500 / FilterCond_Material)
    FILT_CONTENTS  = 4,   // brush/face contents bits      (sub_411580 / sub_412470)
    FILT_KEYVALUE  = 5,   // entity epair key/value        (sub_411450 / FilterCond_Surface)
    FILT_MISC      = 6,   // world/terrain/curve/decal     (sub_4113D0 / sub_412320)
    FILT_SURFACE   = 7,   // surface flag bits             (sub_4116B0 / sub_412500)
    FILT_ECLASS    = 8,   // eclass classtype flag bits    (sub_411630 / sub_4124D0)
};

// _GetToken (0x4B83E0) — read the next ON-LINE token (== Com_ParseOnLine; see idiom).
static inline parseInfo_t *Filter_GetToken( const char **text ) { return Com_ParseOnLine( text ); }

// Allocate a 16-byte filter_info_s (12 named bytes + the +12 sibling link), set type
// + negate, zero contents + sibling.  The contents node is allocated by the caller.
static filter_info_s *Filter_NewInfo( int type )
{
    filter_info_s *n = (filter_info_s *)operator new( 0x10u );
    n->surfaceFlags    = type;        // [n]      condition type
    n->z_pad_0x0004[0] = 0;           // [n+4]    negate flag (set by caller)
    n->contents_ptr    = nullptr;     // [n+8]    contents/children
    *((void **)n + 3)  = nullptr;     // [n+0xC]  next-sibling link (leaf → 0)
    return n;
}

// ── Texture [All] [*]<name>  (sub_411500, 0x411500) → case 3 ──────────────────
// contents node: 8 bytes {char* name@0, int flags@4 (bit0 = `*` leading wildcard,
// bit1 = "All" — match any face)}.
static filter_info_s *Filter_ParseTexture( const char **text )
{
    filter_info_s *n = Filter_NewInfo( FILT_TEXTURE );
    int *c = (int *)operator new( 8u );
    n->contents_ptr = (filter_contents_s *)c;
    c[0] = 0; c[1] = 0;
    parseInfo_t *t = Filter_GetToken( text );
    if ( !_stricmp( t->token, "all" ) ) { c[1] |= 2; t = Filter_GetToken( text ); }
    if ( t->token[0] == '*' )           { c[1] |= 1; t = Filter_GetToken( text ); }
    c[0] = (int)AllocMaterialString( t->token );
    return n;
}

// ── name→bit tables (dumped verbatim from the IDB: off_73C388/73C498/73C520) ──
// Each is an interleaved {const char* name; int bits} pair table; the binary indexes
// it at stride 2 dwords (name@8*i, bits@8*i+4).  We store proper pairs and iterate.
struct FilterFlagName { const char *name; int bits; };
static const FilterFlagName s_contentsTbl[] = {   // 0x73C388 — 34 pairs
    {"solid",0x1},{"foliage",0x2},{"ai_avoid",0x4},{"vehicletrigger",0x8},{"glass",0x10},
    {"water",0x20},{"canshootclip",0x40},{"missileclip",0x80},{"item",0x100},
    {"vehicleclip",0x200},{"itemclip",0x400},{"sky",0x800},{"ai_nosight",0x1000},
    {"clipshot",0x2000},{"actor",0x4000},{"fake_actor",0x8000},{"playerclip",0x10000},
    {"monsterclip",0x20000},{"axistrigger",0x40000},{"alliestrigger",0x80000},
    {"neutraltrigger",0x100000},{"use",0x200000},{"nonsentienttrigger",0x400000},
    {"vehicle",0x800000},{"mantle",0x1000000},{"player",0x2000000},{"corpse",0x4000000},
    {"detail",0x8000000},{"structural",0x10000000},{"lookat",0x20000000},
    {"translucent",0x20000000},{"playertrigger",0x40000000},{"nodrop",(int)0x80000000},
    {"noncolliding",0x4},
};
static const FilterFlagName s_surfaceTbl[] = {    // 0x73C498 — 17 pairs
    {"nodamage",0x1},{"slick",0x2},{"sky",0x4},{"ladder",0x8},{"noimpact",0x10},
    {"nomarks",0x20},{"nopenetrate",0x100},{"hdrportal",0x40},{"nodraw",0x80},
    {"nolightmap",0x400},{"nosteps",0x2000},{"nonsolid",0x4000},{"nodlight",0x20000},
    {"nocastshadow",0x40000},{"mantleon",0x2000000},{"mantleover",0x4000000},
    {"portal",(int)0x80000000},
};
static const FilterFlagName s_eclassTbl[] = {     // 0x73C520 — 9 pairs
    {"light",0x1},{"angle",0x2},{"path",0x4},{"miscmodel",0x8},{"prefab",0x10},
    {"node",0x20},{"triggerradius",0x40},{"triggerdisk",0x80},{"reflection_probe",0x100},
};
static int Filter_LookupBits( const FilterFlagName *tbl, int n, const char *name )
{
    int bits = 0;   // 0 if no name matches (IDA: the field stays its initial 0)
    for ( int i = 0; i < n; ++i )
        if ( !_stricmp( name, tbl[i].name ) )
            bits = tbl[i].bits;
    return bits;
}

// ── Contents [All] <name>  (sub_411580, 0x411580) → case 4 ────────────────────
// contents node: 8 bytes {int contentBits@0, int flags@4 (bit1=All)}; <name> mapped
// to a bit via the 34-entry table.
static filter_info_s *Filter_ParseContents( const char **text )
{
    filter_info_s *n = Filter_NewInfo( FILT_CONTENTS );
    int *c = (int *)operator new( 8u );
    n->contents_ptr = (filter_contents_s *)c;
    c[0] = 0; c[1] = 0;
    parseInfo_t *t = Filter_GetToken( text );
    if ( !_stricmp( t->token, "all" ) ) { c[1] |= 2; t = Filter_GetToken( text ); }
    char *nm = (char *)AllocMaterialString( t->token );
    _strlwr( nm );
    c[0] = Filter_LookupBits( s_contentsTbl, 34, nm );
    free( nm );
    return n;
}

// ── EClassFlag <name>  (sub_411630, 0x411630) → case 8 ────────────────────────
// contents node: 8 bytes {int eclassBits@0}; <name> mapped via the 9-entry table.
static filter_info_s *Filter_ParseEClassFlag( const char **text )
{
    filter_info_s *n = Filter_NewInfo( FILT_ECLASS );
    int *c = (int *)operator new( 8u );
    n->contents_ptr = (filter_contents_s *)c;
    c[0] = 0; c[1] = 0;
    parseInfo_t *t = Filter_GetToken( text );
    char *nm = (char *)AllocMaterialString( t->token );
    _strlwr( nm );
    c[0] = Filter_LookupBits( s_eclassTbl, 9, nm );
    free( nm );
    return n;
}

// ── Surface [All] <name>  (sub_4116B0, 0x4116B0) → case 7 ─────────────────────
// contents node: 8 bytes {int surfaceBits@0, int flags@4 (bit1=All)}.
static filter_info_s *Filter_ParseSurface( const char **text )
{
    filter_info_s *n = Filter_NewInfo( FILT_SURFACE );
    int *c = (int *)operator new( 8u );
    n->contents_ptr = (filter_contents_s *)c;
    c[0] = 0; c[1] = 0;
    parseInfo_t *t = Filter_GetToken( text );
    if ( !_stricmp( t->token, "all" ) ) { c[1] |= 2; t = Filter_GetToken( text ); }
    char *nm = (char *)AllocMaterialString( t->token );
    _strlwr( nm );
    c[0] = Filter_LookupBits( s_surfaceTbl, 17, nm );
    free( nm );
    return n;
}

// ── KeyValue <key> <value>[*]  (sub_411450, 0x411450) → case 5 ────────────────
// contents node: 12 bytes {char* key@0, char* value@4, int flags@8 (bit0 = `*`
// trailing-wildcard prefix match)}.  Read key + value on the line; if the next token
// is a bare `*`, set the wildcard bit, else unget it.
static filter_info_s *Filter_ParseKeyValue( const char **text )
{
    filter_info_s *n = Filter_NewInfo( FILT_KEYVALUE );
    int *c = (int *)operator new( 0xCu );
    n->contents_ptr = (filter_contents_s *)c;
    c[2] = 0;
    parseInfo_t *t = Filter_GetToken( text );
    c[0] = (int)AllocMaterialString( t->token );
    t = Filter_GetToken( text );
    c[1] = (int)AllocMaterialString( t->token );
    if ( Filter_GetToken( text )->token[0] == '*' )
        c[2] |= 1;
    else
        Com_UngetToken();                         // value had no trailing `*`
    return n;
}

// ── Misc <world|terrain|curve|decal>  (sub_4113D0, 0x4113D0) → case 6 ─────────
// contents node: 8 bytes {char* name@0 (lowercased)}.
static filter_info_s *Filter_ParseMisc( const char **text )
{
    filter_info_s *n = Filter_NewInfo( FILT_MISC );
    int *c = (int *)operator new( 8u );
    n->contents_ptr = (filter_contents_s *)c;
    c[1] = 0;
    parseInfo_t *t = Filter_GetToken( text );
    if ( !_stricmp( t->token, "all" ) ) { c[1] |= 2; t = Filter_GetToken( text ); }
    c[0] = (int)AllocMaterialString( t->token );
    _strlwr( (char *)c[0] );
    return n;
}

// ── DynamicFilter_ParseCondition (0x411760) — recursive-descent over the grammar.
// A leading `!` sets the negate flag (written into result[+4]).  A `(` opens a list:
// parse children, joining with `&&` (type 1) / `||` (type 2); the list node's type is
// set to whichever operator separates the children (the binary stores it on the LAST
// `&&`/`||` seen, exactly as transcribed).  Otherwise dispatch on the leaf keyword.
filter_info_s *DynamicFilter_ParseCondition( const char **text )
{
    parseInfo_t *t = Com_Parse( text );      // prologue + Com_ParseExt(.,1)
    bool negate = false;
    if ( t->token[0] == '!' ) { negate = true; t = Com_Parse( text ); }

    if ( t->token[0] == '(' )
    {
        filter_info_s *list = Filter_NewInfo( 0 );  // type set by the first operator
        list->z_pad_0x0004[0] = (char)negate;       // result[4] = negate
        while ( true )
        {
            filter_info_s *child = DynamicFilter_ParseCondition( text );
            // Prepend the child to list->contents_ptr; the sibling link is child[+12].
            *((void **)child + 3) = list->contents_ptr;
            list->contents_ptr    = (filter_contents_s *)child;
            t = Com_Parse( text );
            if ( !strcmp( t->token, "&&" ) ) { list->surfaceFlags = FILT_AND; continue; }
            if ( !strcmp( t->token, "||" ) ) { list->surfaceFlags = FILT_OR;  continue; }
            break;   // expect `)`
        }
        return list;
    }

    filter_info_s *leaf = nullptr;
    if      ( !_stricmp( t->token, "texture" )    ) leaf = Filter_ParseTexture( text );
    else if ( !_stricmp( t->token, "contents" )   ) leaf = Filter_ParseContents( text );
    else if ( !_stricmp( t->token, "eclassflag" ) ) leaf = Filter_ParseEClassFlag( text );
    else if ( !_stricmp( t->token, "surface" )    ) leaf = Filter_ParseSurface( text );
    else if ( !_stricmp( t->token, "keyvalue" )   ) leaf = Filter_ParseKeyValue( text );
    else if ( !_stricmp( t->token, "misc" )       ) leaf = Filter_ParseMisc( text );

    if ( !leaf )
        Com_Error( ERR_FATAL, "DynamicFilter_ParseCondition: failed" );
    leaf->z_pad_0x0004[0] = (char)negate;    // result[4] = negate
    return leaf;
}

// ── qe3_cpp_01 (0x411280) — parse a whitespace-separated material-name LIST for a
// `face`-type filter entry (the body between { } when filter_type_enum & 4).  Each
// node is 8 bytes {char* name@0, void* next@4}, prepended.  Reads the first token
// on-line (after the `{`, space-delimited off), then subsequent tokens until empty.
// Returns the list head (or null if the list is empty).
struct FaceTexNode { void *name; FaceTexNode *next; };
FaceTexNode *qe3_cpp_01( const char **text )
{
    FaceTexNode *head = nullptr;
    parseInfo_t *t = Com_Parse( text );      // prologue + Com_ParseExt(.,1)
    if ( !strlen( t->token ) )
        return nullptr;
    while ( true )
    {
        FaceTexNode *node = (FaceTexNode *)operator new( 8u );
        node->name = AllocMaterialString( t->token );
        node->next = head;
        head = node;
        t = Filter_GetToken( text );          // _GetToken == Com_ParseOnLine
        if ( !strlen( t->token ) )
            return head;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterCondition_Eval (sub_412170, 0x412170)
// Recursively evaluates a filter condition tree against a brush definition.
// AND (1) and OR (2) recurse; cases 3..8 evaluate the six leaf forms.
// ─────────────────────────────────────────────────────────────────────────────
static bool FilterCondition_Eval(brush_t *a1, filter_info_s *a2);

// Cases 3..8 are Texture, Contents, KeyValue, Misc, Surface, and EClassFlag.
static bool FilterCond_Material  (brush_t *a1, filter_info_s *a2); // sub_412630
static bool FilterCond_Contents  (brush_t *a1, filter_info_s *a2); // sub_412470
static bool FilterCond_Surface   (brush_t *a1, filter_info_s *a2); // sub_412290 KeyValue
static bool FilterCond_Misc      (brush_t *a1, filter_info_s *a2); // sub_412320
static bool FilterCond_SurfaceFlag(brush_t *a1, filter_info_s *a2);// sub_412500
static bool FilterCond_EClassFlag(brush_t *a1, filter_info_s *a2); // sub_4124D0

static bool FilterCondition_Eval(brush_t *a1, filter_info_s *a2)
{
    // IDA 0x412170 — switch on a2->surfaceFlags (the condition type)
    switch ( a2->surfaceFlags )
    {
    case 1: // AND — all children must match
    {
        filter_contents_s *child = a2->contents_ptr;
        if ( !child )
            return a2->z_pad_0x0004[0] == 0;
        do {
            if ( !FilterCondition_Eval(a1, (filter_info_s *)child) )
                return (bool)a2->z_pad_0x0004[0];
            child = (filter_contents_s *)((filter_info_s *)child)[1].surfaceFlags;
        } while ( child );
        return a2->z_pad_0x0004[0] == 0;
    }
    case 2: // OR — any child must match
    {
        filter_contents_s *child = a2->contents_ptr;
        if ( !child )
            return (bool)a2->z_pad_0x0004[0];
        do {
            if ( FilterCondition_Eval(a1, (filter_info_s *)child) )
                return a2->z_pad_0x0004[0] == 0;
            child = (filter_contents_s *)((filter_info_s *)child)[1].surfaceFlags;
        } while ( child );
        return (bool)a2->z_pad_0x0004[0];
    }
    case 3: // Texture / material-name match (sub_412630)
        return FilterCond_Material(a1, a2);
    case 4: // Contents flag bits (sub_412470)
        return FilterCond_Contents(a1, a2);
    case 5: // KeyValue epair match (sub_412290)
        return FilterCond_Surface(a1, a2);
    case 6: // Misc world/terrain/curve/decal (sub_412320)
        return FilterCond_Misc(a1, a2);
    case 7: // Surface flag bits (sub_412500)
        return FilterCond_SurfaceFlag(a1, a2);
    case 8: // EClassFlag bits (sub_4124D0)
        return FilterCond_EClassFlag(a1, a2);
    default:
        return false;
    }
}

// case 3 — Texture/material-name match (sub_412630).
// Materialdef_GetName returns the radiant material name for ordinary world
// materials and the layered-material name for layered ones. IDA 0x412630:
//   • NOT "All" (node.flags&2==0) and NOT a patch → per-face: compare every base-layer face's
//     material name against node.name (node.flags&1 → substring/strstr, else exact/strcmp); the
//     brush matches if ANY face matches.
//   • "All" set, or a patch brush → use the brush's representative material (patch->texture for
//     a patch; else brush.xx1, the first-face mtldef cached when all faces share contents — 0 if
//     heterogeneous) and compare once (sub_4125E0: same wildcard/exact rule).
// Match → !negate; no match → negate (faithful to *(a2+4)==0 / *(a2+4)).
static bool FilterCond_Material(brush_t *a1, filter_info_s *a2)
{
    const int   flags  = ((int *)a2->contents_ptr)[1];               // node.flags (bit0=`*`, bit1=All)
    const char *target = (const char *)((void **)a2->contents_ptr)[0]; // node.name
    bool        negate = (a2->z_pad_0x0004[0] != 0);

    if ( ( flags & 2 ) == 0 && !a1->patch )
    {
        if ( !a1->faceCount )
            return negate;
        for ( int i = 0; i < a1->faceCount; ++i )
        {
            MaterialDef *md = &a1->faces[i].mtldef[0];
            if ( ( md->lyrMtl != nullptr ) + ( md->radMtl != nullptr ) != 1 )  // MtlDef_IsValid
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                        85, 0, "%s", "MtlDef_IsValid( mtlDef )" );
            const char *name = (const char *)Materialdef_GetName( md );
            bool hit = ( flags & 1 ) ? ( strstr( name, target ) != nullptr )
                                     : ( strcmp( name, target ) == 0 );
            if ( hit )
                return !negate;
        }
        return negate;
    }

    // "All" or patch: one representative material for the whole brush.
    MaterialDef *md = a1->patch ? (MaterialDef *)&a1->patch->texture
                                : (MaterialDef *)(intptr_t)a1->xx1;
    if ( !md )
        return negate;
    const char *name = (const char *)Materialdef_GetName( md );
    bool hit = ( flags & 1 ) ? ( strstr( name, target ) != nullptr )
                             : ( strcmp( name, target ) == 0 );
    return hit ? !negate : negate;
}

// ── case 4 — Contents flag bits (sub_412470, 0x412470) ────────────────────────
// Matches when the brush has a face
// whose combined contents (brush.contents | face.contents) does NOT intersect the
// filter's content bits — i.e. there exists a face that is NOT excluded by these bits.
// IDA: a1[15]=brush.contents(@0x3C), a1[16]=faceCount(@0x40), a1[17]=faces(@0x44);
// faces at +180(face.contents) stride 232; loop while ((brushC|faceC)&bits)!=0.
static bool FilterCond_Contents(brush_t *a1, filter_info_s *a2)
{
    int *cond      = (int *)a2->contents_ptr;     // {contentBits@0, flags@4}
    int  bits      = cond[0];
    bool negate    = (a2->z_pad_0x0004[0] != 0);
    unsigned int faceCount = (unsigned int)a1->faceCount;
    if ( !faceCount )
        return !negate;                            // no faces → "matches" (IDA returns !negate)
    int   brushContents = a1->contents;
    char *face = (char *)a1->faces;
    for ( unsigned int i = 0; ; ++i, face += sizeof(face_t) )
    {
        int faceContents = *(int *)( face + offsetof(face_t, contents) );  // +180
        if ( ( ( brushContents | faceContents ) & bits ) == 0 )
            return negate;                         // a face NOT excluded by these bits
        if ( i + 1 >= faceCount )
            return !negate;                        // every face is excluded
    }
}
// ── case 5: key/value condition (sub_412290, 0x412290) ───────────────────────
// This is what the LAYER filter (layers.cpp Layers_BuildFilter, sub_411950) uses:
// a filter_contents_s with key="script_layer" and value=<layer name>, flags=0.
// flags bit0 set → walk the entity's epairs by hand and strncmp the value (literal
// prefix compare path); flags clear (our layer case) → Entity_HasEpairMatch exact.
// `a1` is the brush DEFINITION; `a1->owner` is the brush's owning entity (whose
// epairs carry the script_layer key).  z_pad_0x0004[0] is the negation flag: when
// it matches, the result is `negate==0` (true unless negated), else `negate`.
static bool FilterCond_Surface(brush_t *a1, filter_info_s *a2)
{
    // IDA 0x412290 — sub_412290(a1@<ecx>=brush def, a2@<ebx>=filter_info_s)
    filter_contents_s *cond = a2->contents_ptr;
    const char       *key   = cond->key;            // v3 = *(char**)v2
    bool              negate = (a2->z_pad_0x0004[0] != 0);

    if ( (cond->flags & 1) != 0 )
    {
        // Literal path: walk the owning entity's epairs, find `key`, prefix-compare.
        const char *val = "";                       // IDA: v5 = zero
        epair_t *ep = a1->owner ? a1->owner->epairs : nullptr;  // *(a1+8)+116
        for ( ; ep; ep = ep->next )
            if ( _stricmp( ep->key, key ) == 0 ) { val = ep->value; break; }
        if ( !strncmp( val, cond->value, strlen( cond->value ) ) )
            return !negate;
        return negate;
    }

    // Epair-exact path (the layer-filter case): entity must have key==value.
    if ( Entity_HasEpairMatch( a1->owner, key, cond->value ) )
        return !negate;
    return negate;
}
// case 6 — Misc world/terrain/curve/decal (sub_412320).
// IDA 0x412320, faithful transcription:
//   world   — brush owned by the worldspawn (owner == world_entity->def).
//   decal   — any base-layer face whose locale bits intersect the "decal" locale bit.
//   terrain — patch brush whose patch->type == 64 (the terrain discriminant).
//   curve   — patch brush whose patch->type != 64.
// world/terrain/curve are pure struct reads and FULLY faithful.  decal walks faces through
// the now-real MaterialDef_02/MaterialDef_05 realize path; the SEED is texWndGlob.unk_8
// (1<<localeIndex("decal"), set by the unported P6 FillTextureMenu — 0 in kisak) and the
// per-material locale field (qtexture.tex_num_or_localefilter) is also not retained by kisak
// (Editor_AddRadiantMaterial leaves it 0 — CoD4 locale metadata is dropped).  So `Misc decal`
// degenerates to "matches nothing" — the CODE is faithful; the DATA (locale bits) is a known,
// documented kisak limitation, identical to the texwnd locale-filter simplification.  This is
// SAFE (a decal filter just hides nothing), not a logic error.  See PROGRESS material-P3.
static bool FilterCond_Misc(brush_t *a1, filter_info_s *a2)
{
    const char *node = (const char *)((void **)a2->contents_ptr)[0];  // {char* name@0, flags@4}
    const int   flags = ((int *)a2->contents_ptr)[1];
    bool        negate = (a2->z_pad_0x0004[0] != 0);

    if ( !strcmp( node, "world" )
         && (void *)a1->owner == (world_entity ? world_entity->def : nullptr) )
        return !negate;

    if ( !strcmp( node, "decal" ) )
    {
        int matchCount = 0;
        for ( int i = 0; i < a1->faceCount; ++i )
        {
            MaterialDef *md = &a1->faces[i].mtldef[0];
            if ( !Materialdef_Realize( md ) )
                continue;
            dword_181F51C = TexWnd_GetDecalLocaleBit();         // texWndGlob.unk_8 seed
            MaterialDef_02( md, MaterialDef_05 );
            if ( dword_181F51C )
                ++matchCount;
        }
        if ( matchCount && ( matchCount == a1->faceCount || ( flags & 2 ) == 0 ) )
            return !negate;
        // fall through to terrain/curve tests (IDA LABEL_15) — a brush is not both.
    }

    if ( !strcmp( node, "terrain" ) && a1->patch && (int)a1->patch->type == 64 )
        return !negate;
    if ( !strcmp( node, "curve" )   && a1->patch && (int)a1->patch->type != 64 )
        return !negate;
    return negate;
}

// case 7 — Surface flag bits (sub_412500). MaterialDef_02 /
// MaterialDef_09 are fully ported (materialdef.cpp), and Editor_AddRadiantMaterial now
// populates qtexture_s.color_or_surfacetype_filter (@0x1C) from the registered material's
// MaterialInfo.surfaceTypeBits — so the realize callback (MaterialDef_09 ANDs that field
// into dword_181F51C) reads surface-type bits. IDA 0x412500:
//   patch brush      → seed dword_181F51C=surfaceBits, realize patch->texture[0]; matched
//                      iff bits survive the AND.
//   plain brush      → realize each base-layer face; count how many keep their bits.  The
//                      brush matches when EVERY face matched, OR (when "All" is NOT set,
//                      node.flags&2==0) when ANY face matched.
//   no faces         → no match (return negate).
// Match result: when matched, return !negate; else negate (faithful to *(a2+4)==0 / *(a2+4)).
static bool FilterCond_SurfaceFlag(brush_t *a1, filter_info_s *a2)
{
    int  *node       = (int *)a2->contents_ptr;     // {surfaceBits@0, flags@4 (bit1=All)}
    int   surfaceBits = node[0];
    bool  negate     = (a2->z_pad_0x0004[0] != 0);

    if ( a1->patch )                                // a1[20] = patch
    {
        MaterialDef *md = (MaterialDef *)&a1->patch->texture;
        if ( !Materialdef_Realize( md ) )
            return negate;
        dword_181F51C = surfaceBits;                // seed; MaterialDef_09 ANDs surfaceTypeBits
        MaterialDef_02( md, MaterialDef_09 );        // patch+24 (IDA)
        return dword_181F51C ? !negate : negate;
    }
    if ( !a1->faceCount )
        return negate;

    int matchCount = 0;
    for ( int i = 0; i < a1->faceCount; ++i )
    {
        MaterialDef *md = &a1->faces[i].mtldef[0];
        if ( !Materialdef_Realize( md ) )
            continue;
        dword_181F51C = surfaceBits;
        MaterialDef_02( md, MaterialDef_09 );
        if ( dword_181F51C )
            ++matchCount;
    }
    // matched if all faces matched, or (when "All" not set) any face matched.
    if ( matchCount && ( matchCount == a1->faceCount || ( node[1] & 2 ) == 0 ) )
        return !negate;
    return negate;
}

// ── case 8 — EClassFlag bits (sub_4124D0, 0x4124D0) ───────────────────────────
// Reads the owning entity's eclass classtype flags. IDA arg order is
// swapped (sub_4124D0(a2=info@eax, a1=brush@ecx)); transcribed to (brush, info) here.
// Match when (brush->owner->eclass->classtype & filterEclassBits) != 0.
static bool FilterCond_EClassFlag(brush_t *a1, filter_info_s *a2)
{
    int *cond   = (int *)a2->contents_ptr;          // {eclassBits@0}
    bool negate = (a2->z_pad_0x0004[0] != 0);
    // IDA: *(*(*(brush+8)+96)+384) & **(info+8)
    //      brush+8=owner, +96=eclass, eclass+384=classtype; info+8=contents, [0]=bits.
    int classtype = a1->owner->eclass->classtype;
    if ( ( classtype & cond[0] ) != 0 )
        return !negate;
    return negate;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterBrush_CheckFilterListHide (sub_412050, 0x412050)
// Walks a filter_entry_s list and returns true if any non-face, non-fdshow entry
// matches the brush def (i.e., the brush would be HIDDEN by that filter).
// ─────────────────────────────────────────────────────────────────────────────
static bool FilterBrush_CheckFilterListHide(filter_entry_s *list, brush_t *def)
{
    // IDA 0x412050 — sub_412050
    // While condition in IDA: keep looping if (isShown) || (face type) || !condition_match
    // i.e., stop and return 1 (hidden) only when: !isShown && !face_type && condition matches
    int v2 = (int)list;
    if ( !v2 )
        return false;
    while ( *(bool *)(v2 + 4)                          // isShown → skip
         || (*(unsigned char *)v2 & 4) != 0             // face-type filter → skip
         || !FilterCondition_Eval(def, *(filter_info_s **)(v2 + 16)) ) // condition not met → skip
    {
        v2 = *(int *)(v2 + 20);                         // ->next_filter
        if ( !v2 )
            return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterBrush_IsFilteredByAnyList (sub_412120, 0x412120)
// Returns true if ANY filter list (geometry/entity/trigger/other/layer) hides
// this brush def.
// ─────────────────────────────────────────────────────────────────────────────
static bool FilterBrush_IsFilteredByAnyList(brush_t *def)
{
    // IDA 0x412120
    return FilterBrush_CheckFilterListHide(g_qeglobals.d_filterGlobals_geometryFilters, def)
        || FilterBrush_CheckFilterListHide(g_qeglobals.d_filterGlobals_entityFilters,   def)
        || FilterBrush_CheckFilterListHide(g_qeglobals.d_filterGlobals_triggerFilters,  def)
        || FilterBrush_CheckFilterListHide(g_qeglobals.d_filterGlobals_otherFilters,    def)
        || FilterBrush_CheckFilterListHide(g_qeglobals.d_filterGlobals_layerFilters,    def);
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterBrush_ApplyFilterList (sub_412090, 0x412090)
// Accumulates fdhide (bit 3) / fdshow (bit 4) flags from a filter list into
// the returned int bitmask.  Called once per category list from FilterBrush.
// ─────────────────────────────────────────────────────────────────────────────
static int FilterBrush_ApplyFilterList(filter_entry_s *list, brush_t *def)
{
    // IDA 0x412090
    int flags = 0;
    for ( filter_entry_s *v = list; v; v = v->next_filter )
    {
        if ( (v->filter_type_enum & 3) != 0 && (v->filter_type_enum & 4) == 0 )
        {
            if ( FilterCondition_Eval(def, v->info) )
            {
                if ( v->filter_type_enum & 2 )
                    flags |= 0x10; // fdshow matched
                else
                    flags |= 0x08; // fdhide matched
            }
        }
    }
    return flags;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterBrush_CheckLayerFaces (sub_46A190, 0x46A190)
// When current_edit_layer == 1, iterates all faces on a brush definition and
// calls MaterialDef_02 to check if any face passes the material realize.
// ─────────────────────────────────────────────────────────────────────────────
static bool FilterBrush_CheckLayerFaces(brush_t *def)
{
    // IDA 0x46A190
    // a1@<ebx> = (int)def (raw brush_t pointer passed in ebx register)
    // *(DWORD*)(a1+64) = def->faceCount   (offset 0x40)
    // *(DWORD*)(a1+68) = (int)def->faces  (offset 0x44)
    // v1 = byte offset into faces, v4 = face index counter
    if ( !def->faceCount )
        return false;

    int v1 = 0; // byte stride from faces base
    int v4 = 0; // face count
    while ( true )
    {
        // v2 = (MaterialDef*)(v1 + faces + 36) = mtldef[0] (the base-layer MaterialDef,
        // at face_t+36 — same offset the FilterCond_* evaluators use) of face at byte offset v1.
        MaterialDef *md = (MaterialDef *)((char *)def->faces + v1 + 36);
        dword_181F51C = 2;
        MaterialDef_02(md, MaterialDef_07);
        if ( dword_181F51C )
            return true;   // face passes layer material test
        v1 += 232;         // sizeof(face_t) = 232
        if ( (unsigned int)(++v4) >= (unsigned int)def->faceCount )
            return false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterBrush (0x46A1F0)
// Main brush visibility/filter check.  Returns non-zero if the brush should be
// hidden / excluded from selection operations.
//
// The result is CACHED in a1->brushFlags (bit 0 = hidden by geometry/entity/etc)
// and invalidated by incrementing g_qeglobals.g_filtersUpdated (which RadiantFilters07
// and Load_RadiantFilters do on filter state change).  a1->xx6 stores the last
// seen filtersUpdated value; a mismatch forces recomputation.
//
// Signature: char FilterBrush(selbrush_t *a1, int a2)
//   a1  = the brush instance node
//   a2  = 0: simple filtered check; non-zero: full hidden+fdshow+fdhide check
// Returns non-zero if the brush is hidden/filtered.
// ─────────────────────────────────────────────────────────────────────────────
char FilterBrush(selbrush_t *a1, int a2)
{
    // IDA 0x46A1F0
    if ( g_qeglobals.current_edit_layer == 1 )
    {
        brush_t *def = a1->def;
        // Fixed-size entities are NOT layer-filtered
        if ( !def->owner->eclass->fixedsize && !FilterBrush_CheckLayerFaces(def) )
            return 1;
    }

    if ( a1->xx6 != g_qeglobals.g_filtersUpdated )
    {
        // Recompute the cached brushFlags filter bits
        bool filtered = FilterBrush_IsFilteredByAnyList(a1->def);

        if ( (g_qeglobals.d_savedinfo.d_xyShowFlags & 0x40) != 0 )
            filtered = !filtered;  // reverse-filter mode

        if ( filtered )
            a1->brushFlags |=  1u;
        else
            a1->brushFlags &= ~1u;

        // Accumulate fdhide/fdshow flags from geometry/entity/trigger/other lists.
        // IDA only uses 4 lists here (not layerFilters) and OR-assigns into brushFlags.
        brush_t *def2 = a1->def;
        int v6 = FilterBrush_ApplyFilterList(g_qeglobals.d_filterGlobals_geometryFilters, def2);
        int v7 = FilterBrush_ApplyFilterList(g_qeglobals.d_filterGlobals_entityFilters,   def2) | v6;
        int v8 = FilterBrush_ApplyFilterList(g_qeglobals.d_filterGlobals_triggerFilters,  def2) | v7;
        a1->brushFlags |= FilterBrush_ApplyFilterList(g_qeglobals.d_filterGlobals_otherFilters, def2) | v8;

        a1->xx6 = g_qeglobals.g_filtersUpdated;
    }

    if ( !a2 )
        return (a1->brushFlags & 5) != 0;

    int brushFlags = a1->brushFlags;
    if ( (brushFlags & 8) != 0 )
        return 1;   // fdhide
    if ( (brushFlags & 0x10) != 0 )
        return (brushFlags & 6) != 0;  // fdshow: also check bit 1/2

    return (a1->brushFlags & 5) != 0;
}

// ═════════════════════════════════════════════════════════════════════════════
//  faceTexMap — the "filter faces by material name" container (IDB global
//  @0x26656A0 head / 0x26656A4 end).  In the binary this is a genuine MSVC 7.1
//  std::map<std::string,int>: a per-material-name REFCOUNT.  A face-type filter
//  entry owns a LIST of material-name substrings (filter_entry_s.material_ptr,
//  built by qe3_cpp_01: {char* name@0; node* next@4}); when that entry is HIDDEN
//  (isShown→0) every name in its list is added (++refcount); when it is SHOWN
//  (isShown→1) every name is removed (--refcount, erase at 0).  The consumer
//  MtlDef_IsFaceFiltered (mayaexport.cpp 0x46FBE0) tests a face's material name
//  against every key by SUBSTRING (strstr) — if any filtered substring occurs in
//  the face material's name, the face is filtered out of the view.
//
// A standard std::map<std::string,int> preserves the required container semantics.
//  (the binary's RB-tree helpers sub_4136E0/413420/4107C0/412880/4129C0/413AC0
//  are just MSVC's <map> codegen).  We mirror the exact add/remove/lookup
//  semantics on a real std::map; no byte-for-byte RB-tree reimplementation is
//  needed or wanted.  The map lives in this TU (the binary's faceTexMap is a
//  private filterGlobals member; nothing else in the port references it directly —
//  the reader reaches it through FaceTexMap_HasSubstringOf below).
// ═════════════════════════════════════════════════════════════════════════════
static std::map<std::string, int> s_faceTexMap;   // 0x26656A0  std::map<std::string,int>

// material-name list node built by qe3_cpp_01 (filter_entry_s.material_ptr chain):
// {char* name@0; FaceTexNode* next@4}.  The IDB types this chain "MaterialInfo*"
// (a misnomer — name@0 / next@4 exactly match the FaceTexNode the parser allocates).
struct FaceTexEntry { const char *name; FaceTexEntry *next; };

// ─────────────────────────────────────────────────────────────────────────────
// RadiantFilters05_large (0x411be0) — INCREMENT each material-name refcount in
//   faceTexMap.  Called on HIDE (RadiantFilters04_isShown: isShown→0) and at load
//   for a face entry that starts unchecked (RadiantFilters 0x411083).  IDA walks
//   a1->material_ptr (the qe3_cpp_01 name list) and for each name does
//   `++faceTexMap[name]` (lower_bound; insert value 1 if absent, else ++the value).
//   Returns the last list node walked (faithful to the binary's `result`).
// ─────────────────────────────────────────────────────────────────────────────
MaterialInfo *RadiantFilters05_large(filter_entry_s *a1)
{
    FaceTexEntry *node = (FaceTexEntry *)a1->material_ptr;   // [a1+8]
    FaceTexEntry *last = node;
    for ( ; node; last = node, node = node->next )
        ++s_faceTexMap[node->name];                          // faithful: ++faceTexMap[name]
    return (MaterialInfo *)last;
}

// ─────────────────────────────────────────────────────────────────────────────
// RadiantFilters06_large (0x411e30) — DECREMENT each material-name refcount in
//   faceTexMap, erasing the entry when it hits 0.  Called on SHOW
//   (RadiantFilters03_isShown: isShown→1) and from RadiantFilters07.  IDA walks
//   a1->material_ptr and for each name: lower_bound (asserts the key is present —
//   "mapIter != ...faceTexMap.end()"), `--*value`, and erase() when it reaches 0.
//   Returns the last list node walked.
// ─────────────────────────────────────────────────────────────────────────────
MaterialInfo *RadiantFilters06_large(filter_entry_s *a1)
{
    FaceTexEntry *node = (FaceTexEntry *)a1->material_ptr;   // [a1+8]
    FaceTexEntry *last = node;
    for ( ; node; last = node, node = node->next )
    {
        std::map<std::string, int>::iterator it = s_faceTexMap.find( node->name );
        if ( it == s_faceTexMap.end() )                      // IDA: Assert mapIter != end()
        {
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\filters.cpp",
                    622, 0, "%s", "mapIter != g_qeglobals.d_filterGlobals.faceTexMap.end()" );
            continue;
        }
        if ( --it->second == 0 )                             // faithful: if(--value==0) erase
            s_faceTexMap.erase( it );
    }
    return (MaterialInfo *)last;
}

// Reader hook for MtlDef_IsFaceFiltered (0x46FBE0, mayaexport.cpp): true iff ANY
// faceTexMap key is a SUBSTRING of `materialName` (the binary's strstr(name, key)
// loop over the whole map).  Empty map ⇒ false (the default/headless case).
bool FaceTexMap_HasSubstringOf( const char *materialName )
{
    if ( !materialName )
        return false;
    for ( std::map<std::string, int>::const_iterator it = s_faceTexMap.begin();
          it != s_faceTexMap.end(); ++it )
        if ( strstr( materialName, it->first.c_str() ) )     // IDA: strstr(faceName, key)
            return true;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// sub_46FCF0 (0x46FCF0) — the picker's face-texture-filter test.  Walk every key in
// faceTexMap and register its render material via Texture_GetHandle; return 1 iff any
// key's registered Material* (qtexture_s.next, +0) equals the picked face's render
// material.  The picker (Test_Ray cycle-pick, sub_48D460) calls this on the hit
// face's material so a face hidden by a Texture filter is not pickable.  Faithful to
// the binary, which iterates the std::map and compares Texture_GetHandle(key)->next
// against a1 (the face material).  Empty map ⇒ 0 (the default/headless case — no face
// filter active, so every face is pickable, exactly as the binary returns).
// ─────────────────────────────────────────────────────────────────────────────
extern qtexture_s *Texture_GetHandle( const char *name );    // texwnd.cpp 0x45a8e0

char sub_46FCF0( Material *faceMtl )
{
    for ( std::map<std::string, int>::const_iterator it = s_faceTexMap.begin();
          it != s_faceTexMap.end(); ++it )
    {
        // Texture_GetHandle copies the key into a 64-byte buffer (strcpy) before lwr;
        // pass a mutable copy so a >=64 key trips the same Com_PrintMessage path.
        if ( Texture_GetHandle( it->first.c_str() )->next == faceMtl )   // ->next == registered Material*
            return 1;
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// RadiantFilters03_isShown (0x411b60, 53 bytes)
// Set a filter entry's isShown = 1 (SHOW/enable it).
// For a face-type filter (bit 2), the material names it owns are no longer hidden,
// so DECREMENT their faceTexMap refcounts (RadiantFilters06_large).
// ─────────────────────────────────────────────────────────────────────────────
void RadiantFilters03_isShown(filter_entry_s *filter)
{
    // IDA 0x411b60 — enable (show) a filter entry; assert it was not already shown.
    iassert( !filter->isShown );
    bool notFaceType = (filter->filter_type_enum & 4) == 0;
    filter->isShown = 1;
    if ( !notFaceType )          // is a face-type filter
        RadiantFilters06_large(filter);  // remove from faceTexMap (no longer filtered)
}

// ─────────────────────────────────────────────────────────────────────────────
// RadiantFilters04_isShown (0x411ba0, 53 bytes)
// Set a filter entry's isShown = 0 (HIDE/disable it).
// For a face-type filter (bit 2), the material names it owns are now hidden, so
// INCREMENT their faceTexMap refcounts (RadiantFilters05_large).
// ─────────────────────────────────────────────────────────────────────────────
void RadiantFilters04_isShown(filter_entry_s *filter)
{
    // IDA 0x411ba0 — disable (hide) a filter entry; assert it was shown.
    iassert( filter->isShown );
    bool notFaceType = (filter->filter_type_enum & 4) == 0;
    filter->isShown = 0;
    if ( !notFaceType )          // is a face-type filter
        RadiantFilters05_large(filter);  // add to faceTexMap (now filtered)
}

// Forward-decl: the recursive condition-tree free (defined with the layer code below).
static void Filters_FreeCondTree( filter_info_s *info );

// ─────────────────────────────────────────────────────────────────────────────
// RadiantFilters (0x410c50, 1330 bytes) — parse a RadiantFilters.txt file and build
// the filter_entry_s linked lists.
//
// Grammar (per block):  <Category> <Name> [fdhide|fdshow|face] { <condition body> }
//   Category ∈ {geometry, trigger, entity, other} → the matching d_filterGlobals_* list.
//   Modifiers: fdhide → type|=1, fdshow → type|=2, face → type|=4 (verbatim 0x410efa).
//   Body: face → qe3_cpp_01 material-name list; else → DynamicFilter_ParseCondition.
//   The binary then reads the registry default (GetProfileIntA "Filters" <name> 1) and
//   sets isShown=0 if the user had unchecked it; this port honours the saved checkbox
//   state via the same key (CWinApp::GetProfileIntA exists in static MFC).
//
// PARSE IDIOM (q_parse, no g_parse symbol): Com_BeginParseSession + Com_Parse (space-
// delimited token reads) / Com_ParseOnLine (the on-line modifier reads) / Com_UngetToken,
// exactly the established radiant idiom (entity.cpp Map_LoadEntities).  spaceDelimited
// is toggled around the { } body so the condition tokens parse identically.
// ─────────────────────────────────────────────────────────────────────────────
int LoadFile( const char *filename, void **bufferptr );             // cmdlib.cpp 0x40ABD0 (C++ linkage, matches eclass.cpp)
extern int CFilterWnd_GetSavedCheck( const char *name );            // registry "Filters\<name>"
static int LoadFile_Resolved( const char *name, void **buf );       // cwd → <fs_basepath>\bin\<name>

static void Filter_SetSpaceDelimited( int on )
{
    ParseThreadInfo *pi = Com_GetParseThreadInfo();
    pi->parseInfo[pi->parseInfoNum].spaceDelimited = (char)on;
}

void RadiantFilters( const char *filterFile )
{
    void *buf = nullptr;
    if ( LoadFile_Resolved( filterFile, &buf ) < 0 || !buf )
        return;                                  // file missing → no filters from it

    const char *text = (const char *)buf;
    Com_BeginParseSession( filterFile );
    Filter_SetSpaceDelimited( 1 );

    while ( true )
    {
        parseInfo_t *cat = Com_Parse( &text );   // category keyword (space-delimited)
        if ( !text )
            break;                               // EOF

        filter_entry_s **listHead;
        if      ( !_stricmp( cat->token, "geometry" ) ) listHead = &g_qeglobals.d_filterGlobals_geometryFilters;
        else if ( !_stricmp( cat->token, "trigger"  ) ) listHead = &g_qeglobals.d_filterGlobals_triggerFilters;
        else if ( !_stricmp( cat->token, "entity"   ) ) listHead = &g_qeglobals.d_filterGlobals_entityFilters;
        else if ( !_stricmp( cat->token, "other"    ) ) listHead = &g_qeglobals.d_filterGlobals_otherFilters;
        else
        {
            Sys_Printf( "unknown filter categegory type '%s' in RadiantFilters.txt\n", cat->token );
            Com_SkipRestOfLine( &text );
            continue;
        }

        // Filter NAME (read on-line, like the binary's Com_ParseExt(.,0) after the category).
        parseInfo_t *nm = Filter_GetToken( &text );
        char nameBuf[256];
        strncpy( nameBuf, nm->token, sizeof( nameBuf ) - 1 );
        nameBuf[sizeof( nameBuf ) - 1] = '\0';

        // Optional modifier tokens (fdhide/fdshow/face), still on the same line, until
        // the line runs out (an empty on-line token), then the binary expects `{`.
        int typeFlags = 0;
        while ( true )
        {
            parseInfo_t *mod = Filter_GetToken( &text );
            if ( !mod->token[0] )
                break;                           // end of line → modifiers done
            if      ( !_stricmp( mod->token, "fdhide" ) ) typeFlags |= 1;
            else if ( !_stricmp( mod->token, "fdshow" ) ) typeFlags |= 2;
            else if ( !_stricmp( mod->token, "face"   ) ) typeFlags |= 4;
            // (any other on-line token is ignored, matching the binary's if/else-if chain)
        }

        // Expect `{` (space-delimited), then parse the body with spaceDelimited=0.
        parseInfo_t *open = Com_Parse( &text );
        if ( strcmp( open->token, "{" ) )
            Com_ScriptErrorDrop( "MatchToken: got '%s', expected '%s'\n", open->token, "{" );
        Filter_SetSpaceDelimited( 0 );

        MaterialInfo  *faceMtl   = nullptr;
        filter_info_s *condTree  = nullptr;
        bool           isFace    = (typeFlags & 4) != 0;
        if ( isFace )
            faceMtl = (MaterialInfo *)qe3_cpp_01( &text );    // material-name list (face filter)
        else
            condTree = DynamicFilter_ParseCondition( &text ); // condition tree

        // Expect `}` (back to space-delimited).
        Filter_SetSpaceDelimited( 1 );
        parseInfo_t *close = Com_Parse( &text );
        if ( strcmp( close->token, "}" ) )
            Com_ScriptErrorDrop( "MatchToken: got '%s', expected '%s'\n", close->token, "}" );

        // Build the entry and prepend to the category list (binary order: *listHead head).
        // IDA 0x410e12: filter_type_enum/name/isShown=1/info=0/next set, then face entries
        // get material_ptr=qe3_cpp_01(...) (0x411022), non-face get info=condTree (0x411033).
        filter_entry_s *e = (filter_entry_s *)operator new( sizeof( filter_entry_s ) );
        e->filter_type_enum = typeFlags;
        e->isShown          = true;              // shown by default
        e->material_ptr     = faceMtl;           // face: qe3_cpp_01 name list; else null
        e->name             = AllocMaterialString( nameBuf );
        e->info             = condTree;
        e->next_filter      = *listHead;
        *listHead           = e;

        // Honour the saved checkbox state (registry "Filters\<name>", default 1=shown).
        // IDA 0x41104e: GetProfileIntA("Filters", name, 1)==0 → isShown=0, and for a FACE
        // entry that starts hidden, RadiantFilters05_large(e) seeds the faceTexMap refcounts.
        if ( !CFilterWnd_GetSavedCheck( e->name ) )
        {
            e->isShown = false;
            if ( isFace )                        // IDA 0x411080-83: !(type&4)==0 → add
                RadiantFilters05_large( e );
        }
    }

    // End the parse session (Com_EndParseSession — pop the parseInfo frame).
    Com_EndParseSession();
    free( buf );
}

// ─────────────────────────────────────────────────────────────────────────────
// Load_RadiantFilters (0x411190, 240 bytes) — load RadiantFilters.txt + the user
// filter file into the (empty-at-startup) filter lists.  Called from CMainFrame::
// OnCreate (0x420940).  The binary just calls RadiantFilters("RadiantFilters.txt"),
// relying on the editor's cwd being the install bin/ dir (where the .exe + the .txt
// sit).  This port can be launched from anywhere (Start-Process sets cwd to the caller),
// so we RESOLVE the path: bare name (cwd, the faithful case) → <fs_basepath>\bin\<name>
// (the known install location — fs_basepath itself is the install root, com_files.cpp).
// ─────────────────────────────────────────────────────────────────────────────
const char *Dvar_GetString( const char *dvarName );              // qcommon (C++ linkage; fs_basepath)

static int LoadFile_Resolved( const char *name, void **buf )
{
    // 1) bare name (cwd) — faithful when the editor runs from the install bin/.
    int n = (int)LoadFile( name, buf );
    if ( n >= 0 && *buf )
        return n;
    // 2) <fs_basepath>\bin\<name> — the install location of RadiantFilters.txt.
    const char *base = Dvar_GetString( "fs_basepath" );
    if ( base && base[0] )
    {
        char path[1024];
        _snprintf( path, sizeof( path ) - 1, "%s\\bin\\%s", base, name );
        path[sizeof( path ) - 1] = '\0';
        n = (int)LoadFile( path, buf );
        if ( n >= 0 && *buf )
            return n;
    }
    return -1;                            // not found in either location
}

void Load_RadiantFilters()        // g_PrefsDlg / prefData_t from prefs.h (included above)
{
    g_qeglobals.g_filtersUpdated = 1;
    RadiantFilters( "RadiantFilters.txt" );

    // The user's extra filter file, if one is configured in preferences.
    if ( g_PrefsDlg && g_PrefsDlg->m_strUserFilterPath.GetLength() > 0 )
        RadiantFilters( (const char *)(LPCSTR)g_PrefsDlg->m_strUserFilterPath );
}

// ─────────────────────────────────────────────────────────────────────────────
// RadiantFilters07 (0x4149c0) — toggle ONE filter entry's isShown (the CFilterWnd
// checklist click).  The binary reads the new check state from the CCheckListBox and
// asserts it differs from the entry's current isShown.  This port exposes the CORE
// (no MFC control coupling) so the hand-built listbox UI can drive it directly:
//   show=true  → enable (RadiantFilters03_isShown tail; face entries touch faceTexMap)
//   show=false → disable (RadiantFilters04_isShown tail)
// then ++g_filtersUpdated (invalidate the per-brush FilterBrush cache) + repaint.
// Non-face entries do not update the face-material refcount map.
// ─────────────────────────────────────────────────────────────────────────────
void RadiantFilters_ToggleEntry( filter_entry_s *cb, bool show )
{
    if ( !cb )
        return;
    bool isFace = (cb->filter_type_enum & 4) != 0;   // (& 4) == 0 → not face (clean)
    cb->isShown = show;
    if ( isFace )
    {
        if ( show ) RadiantFilters06_large( cb );
        else        RadiantFilters05_large( cb );
    }
    ++g_qeglobals.g_filtersUpdated;                  // invalidate the FilterBrush cache
    g_nUpdateBits = -1;                              // repaint all views
}

// ═════════════════════════════════════════════════════════════════════════════
//  CFilterWnd PUBLIC API — what the hand-built Filters inspector pane (win_ent.cpp)
//  drives.  Two control groups, exactly the binary's CFilterWnd:
//    (1) the four category CHECKLISTS (geometry/trigger/entity/other), each a list of
//        the loaded filter_entry_s nodes the user checks/unchecks → RadiantFilters_-
//        ToggleEntry (= RadiantFilters07).  Enumerated via CFilterWnd_GetCategoryHead.
//    (2) the six SIMPLE checkboxes (Angles/Connections/Names/Blocks/Coordinates/Reverse
//        Filter), each a d_xyShowFlags bit — exactly CFilterWnd::GetSettings (0x4140f0)
//        pushes into them, and the same bits the View→Show menu commands flip.  The
//        get/set here mirror that 1:1 (a checked box = the feature SHOWN = bit CLEAR,
//        except ReverseFilter where checked = bit SET — the inverted polarity GetSettings
//        encodes as `(d_xyShowFlags >> 6) & 1`).
// ═════════════════════════════════════════════════════════════════════════════

// Registry "Filters\<name>" saved isShown default (1 = shown).  CWinApp::GetProfileInt
// reads HKCU\...\<app>\Filters; defaults to 1 if absent — matching the binary's
// GetProfileIntA(., "Filters", name, 1) at 0x41104e.  Headless-safe (returns 1 if no app).
int CFilterWnd_GetSavedCheck( const char *name )
{
    CWinApp *app = AfxGetApp();
    if ( !app )
        return 1;                                    // no MFC app (headless) → default shown
    return (int)app->GetProfileIntA( "Filters", name, 1 );
}

// Persist a filter's checkbox state (the binary writes it on dialog close via
// CWinApp::WriteProfileInt; we write on each toggle so it survives a restart).
void CFilterWnd_SaveCheck( const char *name, bool shown )
{
    CWinApp *app = AfxGetApp();
    if ( app )
        app->WriteProfileInt( "Filters", name, shown ? 1 : 0 );
}

// Enumerate: return the head of one category's filter list (0..3 = geo/trig/ent/other),
// matching the binary's CFilterWnd::InitFilterCheckboxes order (sub_414380 over
// geometryFilters/entityFilters/triggerFilters/otherFilters).  Walk ->next_filter.
filter_entry_s *CFilterWnd_GetCategoryHead( int category )
{
    switch ( category )
    {
    case 0: return g_qeglobals.d_filterGlobals_geometryFilters;
    case 1: return g_qeglobals.d_filterGlobals_triggerFilters;
    case 2: return g_qeglobals.d_filterGlobals_entityFilters;
    case 3: return g_qeglobals.d_filterGlobals_otherFilters;
    default: return nullptr;
    }
}

// The six simple show-flag checkboxes (CFilterWnd::GetSettings 0x4140f0).  index →
// (d_xyShowFlags bit, "checked when bit clear?" polarity).  Angles=0x2, Connections=0x4,
// Names=0x8, Blocks=0x10, Coordinates=0x20 are SHOWN when the bit is CLEAR (so checked ==
// !bit); ReverseFilter=0x40 is ON when the bit is SET (checked == bit) — GetSettings'
// `(flags>>6)&1` encodes exactly that inversion.
struct ShowFlagBox { const char *label; int bit; bool checkedWhenClear; };
static const ShowFlagBox s_showFlagBoxes[6] = {
    { "Angles",         0x02, true  },
    { "Connections",    0x04, true  },
    { "Names",          0x08, true  },
    { "Blocks",         0x10, true  },
    { "Coordinates",    0x20, true  },
    { "Reverse Filter", 0x40, false },
};
int  CFilterWnd_ShowFlagCount()                  { return 6; }
const char *CFilterWnd_ShowFlagLabel( int i )    { return (i>=0&&i<6) ? s_showFlagBoxes[i].label : ""; }

// Is checkbox i currently checked (read the d_xyShowFlags bit through its polarity)?
bool CFilterWnd_GetShowFlag( int i )
{
    if ( i < 0 || i >= 6 ) return false;
    int set = (g_qeglobals.d_savedinfo.d_xyShowFlags & s_showFlagBoxes[i].bit) != 0;
    return s_showFlagBoxes[i].checkedWhenClear ? (set == 0) : (set != 0);
}

// Set checkbox i's state → flip the d_xyShowFlags bit accordingly + repaint.  This is the
// programmatic equivalent of the user toggling the box (or the View→Show menu command).
// Every flag has a live draw/filter consumer.
void CFilterWnd_SetShowFlag( int i, bool checked )
{
    if ( i < 0 || i >= 6 ) return;
    const ShowFlagBox &b = s_showFlagBoxes[i];
    bool wantBitSet = b.checkedWhenClear ? !checked : checked;
    if ( wantBitSet ) g_qeglobals.d_savedinfo.d_xyShowFlags |=  b.bit;
    else              g_qeglobals.d_savedinfo.d_xyShowFlags &= ~b.bit;
    ++g_qeglobals.g_filtersUpdated;   // reverse-filter (0x40) feeds FilterBrush's cache
    g_nUpdateBits = -1;               // repaint all views (Names/Coords/Reverse are visible)
}

// ═════════════════════════════════════════════════════════════════════════════
//  LAYER VISIBILITY FILTERS — the LayersDlg → FilterBrush bridge.
//  A layer is "hidden" at runtime by an entry in d_filterGlobals_layerFilters
//  whose condition is `script_layer == <layer name>` (filter case 5) and whose
//  isShown flag is 0.  FilterBrush → FilterBrush_IsFilteredByAnyList →
//  FilterBrush_CheckFilterListHide(layerFilters) → FilterCond_Surface(case 5) →
//  Entity_HasEpairMatch(brush->owner, "script_layer", name).  This is exactly the
//  binary's mechanism; the LayersDlg's CCheckListBox at the bottom maps 1:1 onto
//  these entries.
// ═════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// Layers_BuildFilter (sub_411950, 0x411950) — prepend a script_layer filter entry.
// Allocates filter_entry_s(0x18) + filter_info_s(0x10) + filter_contents_s(0xC),
// wires key="script_layer", value=<name>, surfaceFlags=5 (the case-5 evaluator),
// isShown=1 (visible by default).  Returns the value string (matches IDA's result).
// ─────────────────────────────────────────────────────────────────────────────
const char *Layers_BuildFilter( const char *layerName )
{
    // IDA 0x411950 — operator new(0x18) entry, new(0x10) info, new(0xC) contents.
    filter_entry_s *entry = (filter_entry_s *)operator new( sizeof( filter_entry_s ) );
    entry->name            = AllocMaterialString( layerName );
    entry->isShown         = true;            // v1->isShown = 1
    entry->filter_type_enum= 0;               // not face/fdhide/fdshow — a condition filter
    entry->next_filter     = g_qeglobals.d_filterGlobals_layerFilters;  // prepend
    entry->material_ptr    = nullptr;
    g_qeglobals.d_filterGlobals_layerFilters = entry;

    // The condition node is 16 bytes in the binary (operator new(0x10)) — a 12-byte
    // filter_info_s PLUS a 4th DWORD at +12 (the AND/OR "next sibling" link, read by
    // FilterCondition_Eval as child[1].surfaceFlags and chased by Filters_FreeCondTree
    // as info[3]).  A leaf script_layer node never participates in an AND/OR list, so
    // +12 stays 0; allocate the full 16 + zero +12 to match the binary exactly (and so
    // the recursive free reads a NUL sibling link, not heap garbage).
    filter_info_s *info = (filter_info_s *)operator new( 16 );
    info->surfaceFlags    = 5;                // [esi]    case 5 — script_layer key/value
    info->z_pad_0x0004[0] = 0;                // [esi+4]  negate = 0
    *((void **)info + 3)  = nullptr;          // [esi+0Ch] next-sibling link = 0 (leaf)

    filter_contents_s *cond = (filter_contents_s *)operator new( sizeof( filter_contents_s ) );
    cond->key   = AllocMaterialString( "script_layer" );
    const char *valStr = AllocMaterialString( layerName );
    cond->value = (char *)valStr;
    cond->flags = 0;                          // epair-exact path
    info->contents_ptr = cond;

    entry->info = info;
    return valStr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Filters_FreeCondTree (sub_4119D0, 0x4119D0) — recursively free a condition tree.
// Case 5 (script_layer): free key, value, the contents node, then the info node.
// (The full Filters_Free 0x411ad0 ALSO writes each entry's isShown to the registry
//  via CWinApp::WriteProfileInt — undesirable for the transient, rebuilt-every-
//  refresh layer filters — so the layer path frees without persisting.)
// ─────────────────────────────────────────────────────────────────────────────
extern "C" void free( void * );
static void Filters_FreeCondTree( filter_info_s *info )
{
    // IDA 0x4119D0 — v1 = info; chase the sibling link (info+0xC) per node.
    while ( info )
    {
        filter_info_s *self = info;
        int type = info->surfaceFlags;
        info = (filter_info_s *)*((void **)info + 3);   // v1 = v1[3] (next sibling)
        filter_contents_s *c = self->contents_ptr;       // v2[2]
        switch ( (unsigned)type )
        {
        case 1u: case 2u:                                // AND/OR — recurse children
            Filters_FreeCondTree( (filter_info_s *)c );
            break;
        case 3u: case 6u:
            free( *(void **)c );  free( c );
            break;
        case 4u: case 7u: case 8u:
            free( c );
            break;
        case 5u:                                         // key/value (script_layer)
            free( c->key );  free( c->value );  free( c );
            break;
        default: break;
        }
        free( self );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Layers_FreeVisibilityFilters — free the whole layerFilters list (entry + name +
// condition tree) WITHOUT the registry write, then null the head.  Matches the
// effect of sub_41C200's `Filters_Free(layerFilters); layerFilters = 0;` prologue
// for these synthetic, every-refresh-rebuilt entries.
// ─────────────────────────────────────────────────────────────────────────────
void Layers_FreeVisibilityFilters()
{
    filter_entry_s *f = g_qeglobals.d_filterGlobals_layerFilters;
    while ( f )
    {
        filter_entry_s *next = f->next_filter;
        if ( f->name )
            free( (void *)f->name );
        if ( f->info )
            Filters_FreeCondTree( f->info );             // our entries are non-face (cond tree)
        free( f );
        f = next;
    }
    g_qeglobals.d_filterGlobals_layerFilters = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Layers_FindVisibilityFilter — locate a layer's filter entry by name (the dialog
// queries/toggles ->isShown).  Returns nullptr if the layer carries no brushes
// (so no filter entry was built for it).
// ─────────────────────────────────────────────────────────────────────────────
filter_entry_s *Layers_FindVisibilityFilter( const char *layerName )
{
    for ( filter_entry_s *f = g_qeglobals.d_filterGlobals_layerFilters; f; f = f->next_filter )
        if ( f->name && !strcmp( f->name, layerName ) )
            return f;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Layers_SetLayerVisible — the runtime visibility toggle (RadiantFilters07 tail,
// 0x4149c0).  Sets a layer-filter entry's isShown, invalidates the per-brush filter
// cache (++g_filtersUpdated — every brush re-evaluates FilterBrush) and forces a
// redraw (g_nUpdateBits = -1).  Returns false if the layer has no filter entry.
// ─────────────────────────────────────────────────────────────────────────────
bool Layers_SetLayerVisible( const char *layerName, bool visible )  // g_nUpdateBits declared at top
{
    filter_entry_s *f = Layers_FindVisibilityFilter( layerName );
    if ( !f )
        return false;
    f->isShown = visible;
    ++g_qeglobals.g_filtersUpdated;   // invalidate FilterBrush cache (a1->xx6 != g_filtersUpdated)
    g_nUpdateBits = -1;               // force all views to repaint
    return true;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Radiant_TestFilter — the headless filter_gate driver (RADIANT_SELFTEST <map> filter).
//
//  Exercises the FULL clean filter pipeline that the CFilterWnd checkboxes drive at
//  runtime — WITHOUT the materialdef-coupled global RadiantFilters.txt load:
//    1. Parse a condition STRING via the just-ported DynamicFilter_ParseCondition
//       (the same parser RadiantFilters feeds the `{ ... }` body to).
//    2. Wrap it in a filter_entry_s and prepend to d_filterGlobals_geometryFilters
//       (the exact list FilterBrush walks).  isShown=1 (visible).
//    3. Count active brushes hidden by FilterBrush(b,0) → baseline (the filter is
//       SHOWN, so it hides nothing of its own).
//    4. Toggle isShown=0 (the checkbox UNCHECK = RadiantFilters07/RadiantFilters04
//       path), bump g_filtersUpdated → recount.  Matching brushes are now hidden, so
//       the count RISES by the number of brushes the condition matches.
//    5. Toggle isShown=1, bump again → the count returns to baseline (reversible).
//    6. Free the entry + its condition tree (Filters_FreeCondTree) — leaves the four
//       global lists exactly as found (still empty; Map_SaveFile must be unaffected
//       since filtering is view-only and is NOT serialised to the .map).
//
//  Returns the number of brushes the condition matched (hiddenWhenOff − baseline),
//  or -1 if the toggle was not reversible (baseline != restored) — a hard failure.
//  Only condition types 4 (Contents) and 8 (EClassFlag) are clean (no materialdef);
//  the caller must pass such a condition.
// ═════════════════════════════════════════════════════════════════════════════
extern "C" int strcmp( const char *, const char * );
extern selbrush_t active_brushes;   // map.cpp — the global display list (sentinel node)
static int Filter_CountHidden()
{
    int n = 0;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
        if ( FilterBrush( b, 0 ) )
            ++n;
    return n;
}

// out_baseline / out_off / out_restored receive the three counts for logging.
int Radiant_TestFilter( const char *conditionStr,
                        int *out_baseline, int *out_off, int *out_restored )
{
    // Build the entry.  The condition string is parsed exactly as the filter file body.
    const char *text = conditionStr;
    filter_info_s *info = DynamicFilter_ParseCondition( &text );

    filter_entry_s *entry = (filter_entry_s *)operator new( sizeof( filter_entry_s ) );
    entry->filter_type_enum = 0;          // condition filter (not face/fdhide/fdshow)
    entry->isShown          = true;       // shown → hides nothing of its own
    entry->material_ptr     = nullptr;
    entry->name             = AllocMaterialString( "OpusTestFilter" );
    entry->info             = info;
    entry->next_filter      = g_qeglobals.d_filterGlobals_geometryFilters;
    g_qeglobals.d_filterGlobals_geometryFilters = entry;

    ++g_qeglobals.g_filtersUpdated;       // invalidate the per-brush FilterBrush cache
    int baseline = Filter_CountHidden();

    entry->isShown = false;               // UNCHECK — now matching brushes are hidden
    ++g_qeglobals.g_filtersUpdated;
    int hiddenWhenOff = Filter_CountHidden();

    entry->isShown = true;                // RE-CHECK — restore
    ++g_qeglobals.g_filtersUpdated;
    int restored = Filter_CountHidden();

    if ( out_baseline ) *out_baseline = baseline;
    if ( out_off )      *out_off      = hiddenWhenOff;
    if ( out_restored ) *out_restored = restored;

    // Unlink + free the entry, leaving the global list as found (empty).
    g_qeglobals.d_filterGlobals_geometryFilters = entry->next_filter;
    if ( entry->name ) free( (void *)entry->name );
    Filters_FreeCondTree( entry->info );
    free( entry );
    ++g_qeglobals.g_filtersUpdated;       // re-invalidate so later draws ignore the gone filter

    if ( restored != baseline )
        return -1;                        // not reversible — toggle is broken
    return hiddenWhenOff - baseline;      // # brushes the condition matched
}

// ═════════════════════════════════════════════════════════════════════════════
//  Radiant_TestReverseFilter — the headless reverse-filter (View→Show→Reverse Filter)
//  gate driver.  Proves CMainFrame::OnSelectReverseFilter's deterministic effect: the
//  d_xyShowFlags 0x40 bit, read by FilterBrush (filters.cpp:592), INVERTS each brush's
//  filtered/visible decision.
//
//  Method (no UI):
//    1. Install a condition filter ("Contents detail") with isShown=FALSE so it ACTIVELY
//       HIDES every matching brush (the same active state Radiant_TestFilter measures as
//       "hiddenWhenOff").
//    2. Count brushes hidden by FilterBrush with reverse OFF  → hiddenNormal (= N matching).
//    3. Set d_xyShowFlags |= 0x40, bump g_filtersUpdated (exactly what OnSelectReverseFilter
//       does) → count hidden again → hiddenReverse.
//    4. Clear the bit + bump, recount → must return to hiddenNormal (reversible).
//    5. Tear down the filter, leaving the global lists empty (view-only; .map untouched).
//
//  Returns N (= hiddenNormal, the # brushes the condition matched) on a clean inversion,
//  0 if nothing matched (N/A), or a negative error code:
//    -1  not reversible (clearing 0x40 didn't restore hiddenNormal)
//    -2  inversion broken (hiddenReverse + hiddenNormal != total active brushes)
//  out_total/out_normal/out_reverse receive the counts for logging.
// ═════════════════════════════════════════════════════════════════════════════
static int Filter_CountActive()
{
    int n = 0;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
        ++n;
    return n;
}

int Radiant_TestReverseFilter( const char *conditionStr,
                               int *out_total, int *out_normal, int *out_reverse )
{
    // Build an ACTIVE (isShown=false) condition filter so it hides its matches.
    const char *text = conditionStr;
    filter_info_s *info = DynamicFilter_ParseCondition( &text );

    filter_entry_s *entry = (filter_entry_s *)operator new( sizeof( filter_entry_s ) );
    entry->filter_type_enum = 0;
    entry->isShown          = false;      // ACTIVE → hides matching brushes
    entry->material_ptr     = nullptr;
    entry->name             = AllocMaterialString( "OpusReverseFilter" );
    entry->info             = info;
    entry->next_filter      = g_qeglobals.d_filterGlobals_geometryFilters;
    g_qeglobals.d_filterGlobals_geometryFilters = entry;

    const int totalActive = Filter_CountActive();

    // Save + clear the reverse bit so the baseline is deterministic regardless of state.
    const int savedFlags = g_qeglobals.d_savedinfo.d_xyShowFlags;
    g_qeglobals.d_savedinfo.d_xyShowFlags &= ~0x40;
    ++g_qeglobals.g_filtersUpdated;
    const int hiddenNormal = Filter_CountHidden();

    // Reverse ON (mirrors OnSelectReverseFilter exactly).
    g_qeglobals.d_savedinfo.d_xyShowFlags |= 0x40;
    ++g_qeglobals.g_filtersUpdated;
    const int hiddenReverse = Filter_CountHidden();

    // Reverse OFF again → must restore.
    g_qeglobals.d_savedinfo.d_xyShowFlags &= ~0x40;
    ++g_qeglobals.g_filtersUpdated;
    const int restored = Filter_CountHidden();

    if ( out_total )   *out_total   = totalActive;
    if ( out_normal )  *out_normal  = hiddenNormal;
    if ( out_reverse ) *out_reverse = hiddenReverse;

    // Tear down: unlink + free, restore the original flags, re-invalidate.
    g_qeglobals.d_filterGlobals_geometryFilters = entry->next_filter;
    if ( entry->name ) free( (void *)entry->name );
    Filters_FreeCondTree( entry->info );
    free( entry );
    g_qeglobals.d_savedinfo.d_xyShowFlags = savedFlags;
    ++g_qeglobals.g_filtersUpdated;

    if ( restored != hiddenNormal )
        return -1;                                  // not reversible
    if ( hiddenReverse + hiddenNormal != totalActive )
        return -2;                                  // inversion not a clean complement
    return hiddenNormal;                            // # brushes the condition matched
}
