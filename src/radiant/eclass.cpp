#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\eclass.cpp

#include "stdafx.h"
#include <afxcoll.h>                 // CStringList (Eclass_LoadModel)
#include <io.h>                      // _findfirst64i32 / _findnext64i32 / _findclose
#include <universal/q_parse.h>       // ParseThreadInfo, Com_ParseExt, parseInfo_t
#include <universal/com_files.h>     // FS_ListFiles / FS_FreeFileList / FsListBehavior_e (weapon scan)
#include <gfx_d3d/r_model.h>         // R_RegisterModel

// ── external Radiant/engine function declarations ─────────────────────────────
// These are functions in other Radiant compilation units or the engine subset
// that are not yet declared in any header available to this TU.

// assertive.cpp / qe3.cpp (IDB 0x49CEA0)
void Assert(const char *file, int line, int type, const char *fmt, ...);

// win_qe3.cpp / Sys_* (IDB 0x499E90)
int Sys_Printf(const char *fmt, ...);

// common.cpp (IDB 0x49A9E0)
void FatalError(int code, const char *fmt, ...);

// entity.cpp (IDB 0x483C80)
void Entity_Free_R(entity_s *e);

// brush.cpp (IDB 0x477AC0)
void Brush_BuildWindings(brush_t *b, int bFull);

// select.cpp (IDB 0x494BC0)
void SetupVertexSelection();

// map.cpp (IDB 0x486500) — load entities from file into linked list
int Map_LoadEntities(const char *filename, entity_s *entList, char unknown);

// SilentClientUser / FileExists (IDB 0x4379C0).  Returns int (the real symbol is
// ?FileExists@@YAHPBD@Z); a `bool` decl mangles differently and won't link — this was
// latent until Prefab_Load (which inlines Eclass_RealizeModel→FileExists) was ported.
int FileExists(const char *path);

// qe3.cpp (IDB 0x48B030)
char *AllocMaterialString(const char *src);

// Radiant rendering enable/disable (IDB 0x500700 / 0x5006D0)
void DisableRendering_pp();
void DisableRendering_mm();

// CRT-import wrappers (IDB 0x583A83 / 0x583A8D)
// j__free and j__free_0 are CRT import thunks; map to CRT free() directly.
static inline void j__free(void *p)   { free(p); }
static inline void j__free_0(void *p) { free(p); }

// memcpy_0 is a COMDAT-merged memcpy; use memcpy directly.
#define memcpy_0(d,s,n) memcpy((d),(s),(n))


// Build-list CString helpers (MFC-internal COW string; lpBuffer is the accumulation buffer)
extern void  *lpBuffer;              // IDB 0x25EB238
void cstr(void **pStr, void *init);
void str_set(void **pStr, void *src, int len);
void str_append(void **pStr, void *src, int len);
void __strcpy(void **pStr, int newCap);
void OleException(long hr);
void *cstr_find(void *str, const void *sub);
extern void *zero;                   // IDB 0x6D58F0 — empty CString initialiser

// Parser global (IDB 0x734D50, type ParseThreadInfo from q_parse.h)
// (type available via #include <universal/q_parse.h> above)
extern ParseThreadInfo g_parse;

// cmdlib.cpp (IDB 0x40ABD0) — raw filesystem read into malloc'd buffer
int LoadFile(const char *filename, void **bufferptr);

// FS helpers — declared with C++ linkage to match the kisak build.
// The names match C++ mangling in the kisak link map.
int  FS_ReadFile(const char *path, void **buf);
void Hunk_FreeTempMemory(char *buf);
extern int fs_loadStack;
// fs_searchpaths: forward-declare as void* for a null check only.
// The real definition is searchpath_s* in com_files.cpp (in the radiant build subset).
// Both are pointer-sized; the linker will match via name mangling.
struct searchpath_s;
extern searchpath_s *fs_searchpaths;

// sub_483500 (eclass default attribute setter, IDB 0x483500)
void sub_483500(int listPtr, const char *key, const char *value);

// materialdef.cpp (IDB 0x4315C0) — resolve a material name into a patchMesh_material.
void SetMaterial(const char *name, patchMesh_material *out);

// Prefab loader (IDB 0x480AB0) — load a prefab .map into the model sub-node.
// REAL port below (after Eclass_RealizeModel); called by Eclass_LoadModel for
// classtype&0x10 (CLASS_PREFAB) eclasses.
char Prefab_Load(const char *path, int *node);

// Com_Parse token-buffer parser (IDB sub_48CA40 @ 0x48CA40) — the editor's own
// GetToken. Writes the next whitespace/quote/special-delimited token into g_eclassToken
// (IDB byte_2409CE0 @ 0x2409CE0) and returns the advanced pointer; sets g_eclassTokenEof
// (IDB dword_23F1CDC) at end of input. Used by Eclass_InitFromText.
static char g_eclassToken[1024];          // byte_2409CE0
static int  g_eclassTokenEof = 0;         // dword_23F1CDC
static char *Eclass_GetToken(char *text);

// unknown_libname_321 / _322 (CFile::Write / ~CFile trampolines, IDB 0x596502/0x596993)
void unknown_libname_321(LPCVOID data, DWORD len);
void unknown_libname_322(CFile *f);

// ── globals ────────────────────────────────────────────────────────────────────
// Primary eclass linked list (alphabetized).  Corresponds to GtkRadiant 'eclass'.
eclass_t   *g_eclass     = nullptr;   // 0x025D5B20

// Fallback eclass returned for unknown entity names ("UNKNOWN_CLASS").
// Populated by Eclass_InitForSourceDirectory after scanning all .def files.
eclass_t   *eclass_bad   = nullptr;   // 0x025D5B24  (IDB dword_25D5B24)

// Per-map model eclass list (singly-linked via eclass_t::next cast to int).
// Each entry is a models_t* that has been allocated for a map entity model.
int         g_models     = 0;         // 0x025D5B28  (IDB dword_25D5B28)

// When non-zero, Init_ScanFiles stops after the first /*QUAKED block found.
int         g_eclassStopOnFirst = 0;  // 0x025D5B2C  (IDB dword_25D5B2C)

// Build-list mode: accumulate QUAKED blocks into lpBuffer → newdefs.def.
char        g_bBuildList = 0;         // 0x025D5B05

// Last eclass successfully parsed in Init_ScanFiles.
int         g_lastParsedEclass  = 0;  // 0x023F1728  (IDB dword_23F1728)
// Non-zero once the first /*QUAKED block has been found in the current scan.
int         g_eclassParsedAny   = 0;  // 0x023F172C  (IDB dword_23F172C)
// Points to the name field of the eclass currently being built (for Assert msgs).
int         g_eclassCurrentName = 0;  // 0x023F1730  (IDB dword_23F1730)

// Resolved filepath set by Eclass_InitForSourceDirectory before each
// Init_ScanFiles call (replacing the register-arg calling convention in IDA).
char        g_scanFilePath[1024];     // not an original global; bridges ecx-arg


// ── forward declarations ───────────────────────────────────────────────────────
static void Eclass_FreeEntityList(int modelNode);
static void Eclass_FreeEpairs(int modelNode);
static void Eclass_FreeAll();


// ─────────────────────────────────────────────────────────────────────────────
// sub_4806D0  →  Eclass_FreeEntityList
// Free all entity_s nodes in the circular entity list embedded in a model
// sub-node.  The sub-node layout (all fields are 4-byte words):
//   [+0x00]  void *next_subnode   (next in models_t::x88 linked list)
//   [+0x04 .. +0x07]  (padding / unused)
//   [+0x08]  sentinel back-link  (self when list empty; == &node[2])
//   [+0x0C]  entity_s *entities_next  (first entity in circular list)
//
// The entity circular list is empty when *(node+12) == (node+8).
// For each entity we walk its brush list (brushes.oprev → ... sentinel) and
// decrement ownerPrev on each brush, then decrement entity refCount and free.
// ─────────────────────────────────────────────────────────────────────────────
// Assembly-verified implementation (0x4806D0):
//   edi = a1 (model sub-node ptr)
//   Outer loop: esi = *(edi+C) = first entity in circular list
//   Sentinel: edi+8 (circular list empties when *(edi+C) == edi+8)
//   Brush loop: eax = *(esi+C) = entity.brushes.onext = first brush
//               ecx = esi+8 = entity sub-sentinel
//               add [eax+1Ch], -1 = --brush->refCount  (brush_t.refCount at 0x1C)
//               advance: eax = *(eax+4) = brush->onext
//               loop while eax != esi+8
//   After brushes: add [esi+88h], -1 = --entity->refCount  (entity_s.refCount at 0x88)
//   Entity_Free_R(entity)
static void Eclass_FreeEntityList(int a1)
{
    if ( !*(int *)(a1 + 12) )
    {
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
               39, 0, "%s", "model->entities.next");
    }

    // Loop sentinel: (edi+8); entity ptr at *(edi+C)
    while ( *(int *)(a1 + 12) != a1 + 8 )
    {
        int esi = *(int *)(a1 + 12);  // entity ptr (raw int for offset arithmetic)

        // Check entity refCount == 1 (at esi+0x88)
        if ( *(int *)(esi + 0x88) != 1 )
        {
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
                   43, 1, "%s", "e->refCount == 1");
        }

        // Walk brush circular list: start at entity.brushes.onext (esi+0C),
        // sentinel is esi+8, advance via brush.onext at brush+4.
        // Decrement brush.refCount at brush+0x1C on each brush.
        int eax = *(int *)(esi + 0x0C);  // entity->brushes.onext
        int ecx = esi + 8;               // sub-sentinel
        while ( eax != ecx )
        {
            --*(int *)(eax + 0x1C);       // --brush->refCount
            eax = *(int *)(eax + 4);      // brush = brush->onext
        }

        // Decrement entity refCount (esi+0x88) and free.
        --*(int *)(esi + 0x88);
        Entity_Free_R((entity_s *)esi);
    }

    *(int *)(a1 + 12) = 0;
}


// ─────────────────────────────────────────────────────────────────────────────
// sub_480760  →  Eclass_FreeEpairs
// Free the key/value epair linked list stored at *(modelNode + 380).
// Node layout: [+0] void *next, [+4] char *key, [+8] char *value.
// ─────────────────────────────────────────────────────────────────────────────
static void Eclass_FreeEpairs(int a1)
{
    void **v1 = *(void ***)(a1 + 380);
    if ( v1 )
    {
        void **v2;
        do
        {
            v2 = (void **)*v1;
            j__free_0(v1[1]);
            j__free_0(v1[2]);
            j__free(v1);
            v1 = v2;
        }
        while ( v2 );
    }
    *(int *)(a1 + 380) = 0;
}


// ─────────────────────────────────────────────────────────────────────────────
// Eclass_CheckEntities  (0x4807A0)
// Completely free a models_t node:
//   1. Free every entity-group sub-node in model->x88.
//   2. Free material/string fields (x94, x89, x90-x93).
//   3. Free epairs (via Eclass_FreeEpairs at offset 380).
//   4. Free model->handle and model->x15.
//   5. Free the models_t node itself.
//
// models_t field index mapping (field name → word index → byte offset):
//   x88 = [88] → 0x160,  x94 = [94] → 0x178,  x89 = [89] → 0x164,
//   x90 = [90] → 0x168,  x91 = [91] → 0x16C,  x92 = [92] → 0x170,
//   x93 = [93] → 0x174,  handle = [1] → 0x04,  x15 = [15] → 0x3C
// ─────────────────────────────────────────────────────────────────────────────
void Eclass_CheckEntities(models_t *model)
{
    void *x88 = (void *)model->x88;
    if ( x88 )
    {
        void *v3;
        do
        {
            v3 = *(void **)x88;
            // *(x88+12): entities.next — non-zero means the entity list is populated.
            if ( *(int *)((char *)x88 + 12) )
            {
                Eclass_FreeEntityList((int)x88);
                if ( *(int *)((char *)x88 + 12) )
                {
                    Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
                           88, 1, "%s", "model->entities.next == NULL");
                }
            }
            j__free(x88);
            x88 = v3;
        }
        while ( v3 );
    }

    j__free_0((void *)model->x94);
    j__free_0((void *)model->x89);

    int *p_x90 = &model->x90;
    int  cnt   = 4;
    do
    {
        j__free_0((void *)*p_x90++);
        --cnt;
    }
    while ( cnt );

    Eclass_FreeEpairs((int)model);
    j__free_0((void *)model->handle);
    j__free_0((void *)model->x15);
    j__free(model);
}


// ─────────────────────────────────────────────────────────────────────────────
// sub_480920  →  Eclass_RealizeModel  (0x480920)
// Load a .map/.ent file and attach its entities to the eclass node.
// Returns 1 on success, 0 on failure.
//
// IDA signature: char __usercall sub_480920@<al>(_DWORD *a1@<eax>, const char *a2)
//   a1 = pointer-TO the sub-node pointer (Prefab_Load passes &node); *a1 is the
//        models_t/eclass sub-node, and its entity-list sentinel is at (*a1)+8.
//   a2 = file path.
//
// After loading, for each entity in the list: bump refCount to 1, rebuild brush
// windings (Brush_BuildWindings), and optionally refresh vertex selection.
// ─────────────────────────────────────────────────────────────────────────────
static char Eclass_RealizeModel(int *a1, const char *a2)
{
    // a1 is a POINTER-TO the sub-node pointer (Prefab_Load passes &j). IDA derefs:
    // v3 = *a1 + 8 (the entity-list sentinel), *a1 + 148 = the "file exists" byte.
    // (TASK 3 review fix: the earlier port treated a1 as the node itself and dropped
    //  the deref — a latent crash that surfaces once Prefab_Load is ported.)
    int  node = *a1;
    int *v3   = (int *)( node + 8 );      // circular entity-list sentinel
    int *v7   = v3;
    v3[1] = (int)v3;   // sentinel.next = self
    v3[0] = (int)v3;   // sentinel.prev = self

    // Prefab-load material gate: the prefab's brushes/patches resolve their materials
    // through SetMaterial during the parse below; force the degenerate (wireframe)
    // material path for them so the live CoD4 material-asset loader (broken for CoD4
    // water materials in the CoD3-sized kisak engine — see materialdef.cpp) is not hit.
    // Counter so nested prefab realizes stay degenerate; restored after the parse.
    extern int g_radiantLoadingPrefab;    // materialdef.cpp
    ++g_radiantLoadingPrefab;
    int loadedOk = Map_LoadEntities(a2, (entity_s *)v3, 1);
    --g_radiantLoadingPrefab;
    if ( loadedOk )
    {
        // Optional: mark the eclass if the file also physically exists on disk.
        if ( g_qeglobals.toggle_unk05 && FileExists(a2) )
        {
            *(unsigned char *)( node + 148 ) = 1;
        }

        // Walk all loaded entities (circular list, v7 is sentinel, v5 = current).
        int *v5 = (int *)v3[1];
        if ( v5 != v3 )
        {
            do
            {
                // v5[34] == entity refCount (offset 34*4 = 136 = 0x88)
                if ( ++v5[34] != 1 )
                {
                    Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
                           159, 1, "%s", "e->refCount == 1");
                }

                // Walk brush list: sentinel at v5+2 (offset +8 within entity sub-node,
                // which matches entity->def as the head).
                // v5[3] == brushes.oprev (first brush back-link = offset +12 = brush head)
                for ( brush_t *b = (brush_t *)v5[3];
                      b != (brush_t *)(v5 + 2);
                      b = b->onext )
                {
                    ++b->refCount;
                    Brush_BuildWindings(b, 1);
                    // KISAK: the prefab's brushes just parsed with
                    // DEGENERATE face materials (g_radiantLoadingPrefab forced the shim to
                    // dodge the CoD4-water-material AV).  Now that the parse is done and the
                    // renderer is up, realize each face's material by name — ONCE, here at
                    // load time (outside any draw), so the prefab-content walls render
                    // textured instead of as a white void.  Guarded against an unreadable
                    // CoD4 asset (ERR_DROP → that brush stays degenerate, no crash).  See
                    // brush.cpp Brush_RealizeFaceMaterials for why draw-time realize regressed.
                    extern void Brush_RealizeFaceMaterialsGuarded(brush_t *def);  // engine_stubs.cpp
                    Brush_RealizeFaceMaterialsGuarded(b);
                    if ( g_qeglobals.d_select_mode == sel_vertex ||
                         g_qeglobals.d_select_mode == sel_edge )
                    {
                        SetupVertexSelection();
                    }
                    ++b->version;
                }
                v5 = (int *)v5[1];
            }
            while ( v5 != v7 );
        }
        return 1;
    }
    else
    {
        v3[1] = 0;
        v3[0] = 0;
        return 0;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Prefab_Load  (0x480AB0)  — load a prefab .map and realise its entities into the
// eclass sub-node.
//
// IDA signature: char __usercall Prefab_Load@<al>(char *a1@<edi>, _DWORD *a2)
//   a1 = prefab name (the misc_prefab "model" value, e.g. "prefabs/.../x.map")
//   a2 = the eclass sub-node pointer (Eclass_LoadModel passes &j).
//
// The binary builds an MFC CString from the project entity's "mapspath" value,
// ensures it ends with a backslash, appends the prefab name, then calls
// Eclass_RealizeModel (which Map_LoadEntities's the file into the sub-node and
// bumps refCounts).  Returns Eclass_RealizeModel's result (1 success / 0 fail).
//
// The binary's CString refcount dance (str_set/str_append + InterlockedDecrement
// release) is reproduced 1:1 with a real MFC CString — exactly as Eclass_LoadModel
// (CStringList) and CEntityWnd_OpenModelDialog (CString) do.
// ─────────────────────────────────────────────────────────────────────────────
char Prefab_Load(const char *a1, int *a2)
{
    Sys_Printf( "Loading prefab %s...", a1 );

    // mapspath = the project entity's "mapspath" epair value (empty if absent),
    // matching the IDA: walk d_project_entity->epairs for key "mapspath".
    // The binary derefs d_project_entity->epairs UNCONDITIONALLY (0x480b17). d_project_entity is
    // assigned only in mainfrm.cpp (CMainFrame project load), which the headless selftest gate
    // never runs -> it is NULL there. So this NULL guard is sanctioned headless-safety (same class
    // as the g_pParentWnd guards in map.cpp), NOT an invented behavioral divergence.
    CString path;
    if ( g_qeglobals.d_project_entity )
    {
        for ( epair_t *ep = g_qeglobals.d_project_entity->epairs; ep; ep = ep->next )
            if ( _stricmp( ep->key, "mapspath" ) == 0 ) { path = ep->value; break; }
    }

    // Ensure a trailing backslash (IDA: if length>0 and last char != '\\', append '\\').
    if ( !path.IsEmpty() && path[path.GetLength() - 1] != '\\' )
        path += '\\';

    path += ( a1 ? a1 : "" );

    if ( Eclass_RealizeModel( a2, (const char *)(LPCSTR)path ) )
    {
        Sys_Printf( " successful.\n" );
        return 1;
    }
    Sys_Printf( " failed.\n" );
    return 0;
}


// ─────────────────────────────────────────────────────────────────────────────
// Model_load  (0x480A00)
// Register an XModel by path into (**model)->handle.
// Strips a leading "xmodel/" or "xmodel\" prefix before registering.
//
// IDA signature: LRESULT __usercall Model_load@<eax>(_BYTE *a1@<eax>, int a2@<edi>)
//   a1 = model path string
//   a2 = int* pointing to a models_t** (i.e. *(int*)a2 is the models_t*, field +4 is handle)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT Model_load(char *modelPath, int a2)
{
    int v2 = (int)modelPath;

    if ( !I_strnicmp(modelPath, "xmodel", 6) )
    {
        char v3 = *(char *)(v2 + 6);
        if ( v3 == '/' || v3 == '\\' )
        {
            v2 += 7;
        }
    }

    Sys_Printf("Loading model %s...", (const char *)v2);

    // node = *(int*)a2 (the models_t sub-node); node->handle (+4) = R_RegisterModel(path).
    // HEX-RAYS DOUBLE-DEREF ARTIFACT: hex-rays rendered this as
    //   **(int**)(*(int*)a2 + 4) = ...   (write through node->handle, i.e. *(node->handle))
    // but the disasm is a SINGLE store `mov [ecx+4], eax` (ecx = *a2 = node):
    //   *(int*)(node + 4) = XModel*.  The extra `*` crashed (node->handle is 0 for a fresh
    //   node → write to NULL).  Read-back is `mov esi,[eax+4]` = node->handle, also single.
    int node = *(int *)a2;
    *(int *)( node + 4 ) = (int)R_RegisterModel((char *)v2);

    if ( !*(int *)( node + 4 ) )
    {
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
               179, 0, "%s", "(*model)->handle");
    }

    int v4 = *(int *)( node + 4 );
    if ( !v4 )
    {
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\xanim\\xmodel.cpp",
               48, 0, "%s", "model");
    }

    // CoD3 != CoD4: the IDB reads the bad flag at +193 (CoD4); kisak XModel.bad is +205 ->
    // use XModelBad() (the prefab path doesn't hit this — only xmodel models do).
    extern bool XModelBad( const XModel *model );
    if ( XModelBad( (const XModel *)v4 ) )
    {
        return Sys_Printf(" failed.\n");
    }
    else
    {
        return Sys_Printf(" successful.\n");
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// sub_480CB0  →  Eclass_FreeModel  (0x480CB0)
// Unlink a models_t from g_models and free it entirely.
//
// IDA signature: void __cdecl sub_480CB0(void **a1)
//   a1 = pointer to the models_t to free (same as the node itself cast to void**).
//
// Field word-indices used (each is a 4-byte int field):
//   [0]  = next node in g_models list
//   [88] = x88  head of entity-group sub-node list  (offset 0x160)
//   [89] = x89  (offset 0x164)
//   [90..93] = x90..x93  four material strings (0x168-0x174)
//   [94] = x94  (offset 0x178)
//   [1]  = handle (offset 0x04)
//   [15] = x15   (offset 0x3C)
// ─────────────────────────────────────────────────────────────────────────────
void Eclass_FreeModel(void **a1)
{
    void **v1 = a1;

    // Unlink from the g_models singly-linked list.
    int *ec = (int *)g_models;
    int *i  = &g_models;
    for ( ; ec != (int *)a1; ec = (int *)*ec )
    {
        iassert(ec);
        i = ec;
    }
    *i = *ec;  // splice out

    // Free entity sub-nodes in a1[88] linked list.
    int *v4  = (int *)a1[88];
    int *v11 = v4;
    if ( v4 )
    {
        while ( 1 )
        {
            // v4[3] = entity list sentinel (brushes head), v4[0] = next sub-node
            int *v5  = (int *)v4[3];
            int *v6  = (int *)*v4;
            int *v10 = v6;

            if ( v5 )
            {
                if ( v5 != v4 + 2 )
                {
                    do
                    {
                        // v5[34] = entity refCount (offset 136 = 0x88)
                        if ( (int)v5[34] < 2 )
                        {
                            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
                                   258, 1, "%s", "e->refCount >= 2");
                        }
                        int *v7 = (int *)v5[3];
                        --v5[34];
                        // v7[1] = brush onext, v7[7] = brush refCount (offset 28 = 0x1C)
                        for ( ; v7 != v5 + 2; v7 = (int *)v7[1] )
                        {
                            if ( (int)v7[7] < 2 )
                            {
                                Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
                                       262, 1, "%s", "b->refCount >= 2");
                            }
                            --v7[7];
                        }
                        v5 = (int *)v5[1];
                    }
                    while ( v5 != v11 + 2 );
                    v6  = v10;
                    v4  = v11;
                    v1  = a1;
                }
                v4[3] = 0;
            }
            j__free(v4);
            v11 = v6;
            if ( !v6 )
                break;
            v4 = v6;
        }
    }

    j__free_0(v1[94]);    // x94
    j__free_0(v1[89]);    // x89
    void **v8 = v1 + 90;  // &x90
    int    v9 = 4;
    do
    {
        j__free_0(*v8++);
        --v9;
    }
    while ( v9 );

    j__free_0(v1[1]);     // handle
    j__free_0(v1[15]);    // x15
    j__free(v1);
}


// ─────────────────────────────────────────────────────────────────────────────
// sub_480880  →  Eclass_FreeAll  (internal, called by Eclass_InitForSourceDirectory)
// Free every eclass in g_eclass, every model in g_models, then free eclass_bad.
// ─────────────────────────────────────────────────────────────────────────────
static void Eclass_FreeAll()
{
    models_t *v0 = (models_t *)g_eclass;
    if ( g_eclass )
    {
        eclass_t *x0;
        do
        {
            x0 = (eclass_t *)v0->x0;
            Eclass_CheckEntities(v0);
            v0       = (models_t *)x0;
            g_eclass = x0;
        }
        while ( x0 );
    }

    models_t *v2 = (models_t *)g_models;
    g_eclass = nullptr;
    if ( g_models )
    {
        int v3;
        do
        {
            v3 = v2->x0;
            Eclass_CheckEntities(v2);
            v2       = (models_t *)v3;
            g_models = v3;
        }
        while ( v3 );
    }
    g_models = 0;

    if ( eclass_bad )
    {
        j__free_0(*((void **)eclass_bad + 1));    // eclass_bad->name (field[1])
        j__free_0(*((void **)eclass_bad + 15));   // field[15]
        j__free(eclass_bad);
        eclass_bad = nullptr;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Model_FreeMapModels  (0x480C50)
// Remove and free all g_eclass entries that carry map entity data (x88 != 0),
// then flush g_models entirely.
// ─────────────────────────────────────────────────────────────────────────────
models_t *Model_FreeMapModels()
{
    models_t *v0 = (models_t *)g_eclass;
    eclass_t **v1 = &g_eclass;

    if ( g_eclass )
    {
        do
        {
            if ( v0->x88 )
            {
                *v1 = (eclass_t *)v0->x0;
                Eclass_CheckEntities(v0);
            }
            else
            {
                v1 = (eclass_t **)v0;
            }
            v0 = (models_t *)*v1;
        }
        while ( *v1 );
    }

    models_t *result = (models_t *)g_models;
    if ( g_models )
    {
        models_t *x0;
        do
        {
            x0       = (models_t *)result->x0;
            Eclass_CheckEntities(result);
            result   = x0;
            g_models = (int)x0;
        }
        while ( x0 );
    }
    g_models = 0;
    return result;
}


// ─────────────────────────────────────────────────────────────────────────────
// Init_CStr  (0x480E30)
// Search the eclass comments (ec->comments, field +0x3C) for the attribute 'key'
// (e.g. "model="); if present, copy the quoted value that follows into *out via
// AllocMaterialString. The original is MFC-CString-heavy (cstr/cstr_find/str_set/
// __strcpy/OleException are inlined CStringData ops); with real MFC CString it is
// a few lines. The search key is passed in ecx in the binary (__usercall); the
// six call sites in Eclass_InitFromText pass it explicitly here.
// ─────────────────────────────────────────────────────────────────────────────
static void Init_CStr(eclass_t *ec, void **out, const char *key)
{
    if ( !key )
        return;

    CString comments( ec->comments ? ec->comments : "" );   // cstr(&v11, ec->comments)
    int pos = comments.Find( key );                          // cstr_find(v11, key)
    if ( pos < 0 )
        return;

    // Value text starts just past the key in the ORIGINAL comments string
    // (IDA: ec->comments + strlen(key) + (match - v11)).
    const char *p = ec->comments + pos + strlen( key );
    if ( *p == '"' )            // skip an opening quote if present
        ++p;

    CString value;
    for ( ; *p && *p != '"'; ++p )   // read until closing quote / end
        value += *p;

    if ( value.GetLength() > 0 )
        *out = AllocMaterialString( (LPCSTR)value );
}


// ─────────────────────────────────────────────────────────────────────────────
// sub_483500  (qe3.cpp:56)  — SetKeyValue for the eclass default/epair list.
// listPtr is the ADDRESS of the list-head pointer; nodes are {next, char* key,
// char* value} (12 bytes; freed by Eclass_FreeEpairs). If a node with matching key
// exists, its value is replaced; otherwise a new node is prepended. Ported here
// (qe3.cpp not yet done) because Eclass_ParseDefaults needs it.
// ─────────────────────────────────────────────────────────────────────────────
void sub_483500(int listPtr, const char *key, const char *value)
{
    int **a1  = (int **)listPtr;   // &listHead
    int  *node = *a1;
    while ( node )
    {
        if ( !strcmp( (const char *)node[1], key ) )
        {
            j__free_0( (void *)node[2] );
            if ( !value )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\qe3.cpp", 56, 0, "%s", "str" );
            unsigned int n = (unsigned int)strlen( value );
            void *v = operator new( n + 1 );
            memcpy_0( v, value, n + 1 );
            node[2] = (int)v;
            return;
        }
        if ( !node[0] )
            break;
        node = (int *)node[0];
    }

    // Not found → prepend a new {next, key, value} node.
    int *nn = (int *)operator new( 0xCu );
    nn[0] = (int)*a1;
    *a1   = nn;
    if ( !key )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\qe3.cpp", 56, 0, "%s", "str" );
    unsigned int kn = (unsigned int)strlen( key );
    void *kv = operator new( kn + 1 );
    memcpy_0( kv, key, kn + 1 );
    nn[1] = (int)kv;
    if ( !value )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\qe3.cpp", 56, 0, "%s", "str" );
    unsigned int vn = (unsigned int)strlen( value );
    void *vv = operator new( vn + 1 );
    memcpy_0( vv, value, vn + 1 );
    nn[2] = (int)vv;
}

// ─────────────────────────────────────────────────────────────────────────────
// sub_480F90  ->  Eclass_ParseDefaults  (0x480F90)
// Scan ec->comments (field +0x3C) for "default:" markers; for each, read the next
// two tokens (key, value) with Com_ParseExt and register the default into the
// eclass attribute list at ec+0x17C (via sub_483500). The binary inlines the
// Com_ParseExt unget/backup prologue; calling Com_ParseExt directly is equivalent
// (it performs that unget handling internally on g_parse).
// ─────────────────────────────────────────────────────────────────────────────
static void Eclass_ParseDefaults(eclass_t *ec)
{
    const char *p = strstr( ec->comments, "default:" );
    while ( p )
    {
        const char *text = p + 8;   // past "default:"

        parseInfo_t *kt = Com_ParseExt( &text, 0 );
        if ( !kt || !strlen( kt->token ) )
            Com_PrintMessage( "Invalid default key in .def file!" );
        char key[256];
        strcpy( key, kt ? kt->token : "" );

        parseInfo_t *vt = Com_ParseExt( &text, 0 );
        if ( !vt || !strlen( vt->token ) )
            Com_PrintMessage( "Invalid default value in .def file!" );
        char value[260];
        strcpy( value, vt ? vt->token : "" );

        // ec+0x17C (= &ec->xx8) is the eclass default/epair list head (freed by
        // Eclass_FreeEpairs at +380). IDA: sub_483500(this+95, key, value).
        sub_483500( (int)( (char *)ec + 380 ), key, value );

        p = strstr( text, "default:" );
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Eclass_GetToken  (0x48CA40)  — the editor's COM_Parse/GetToken
// Reads the next token from 'text' into g_eclassToken and returns the advanced
// pointer. Skips whitespace and "//" line comments; reads quoted strings (between
// "), single special chars ({ } ) ( ' :), or whitespace-delimited words. Sets
// g_eclassTokenEof at end of input (returns NULL).
// ─────────────────────────────────────────────────────────────────────────────
static bool IsTokenSpecial( char c )
{
    return c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':';
}

static char *Eclass_GetToken( char *text )
{
    int len = 0;
    g_eclassToken[0] = 0;
    if ( !text )
        return nullptr;

    char c = *text;
    bool singleSlash = false;
    while ( true )
    {
        if ( c <= 32 )                              // skip whitespace
        {
            while ( c ) { c = *++text; if ( c > 32 ) break; }
            if ( !c ) { g_eclassTokenEof = 1; return nullptr; }
        }
        if ( c != '/' )
            break;
        if ( text[1] != '/' ) { singleSlash = true; break; }   // lone '/' → word char
        while ( c && c != '\n' ) c = *++text;       // "//" comment → skip to EOL
        if ( !c ) { g_eclassTokenEof = 1; return nullptr; }
        // c == '\n' (whitespace) — loop re-skips it
    }

    if ( !singleSlash && c == '"' )                 // quoted string
    {
        char q = text[1];
        text += 2;
        while ( q != '"' ) { g_eclassToken[len++] = q; q = *text++; }
        g_eclassToken[len] = 0;
        return text;
    }

    if ( !singleSlash && IsTokenSpecial( c ) )       // single special char
    {
        g_eclassToken[0] = c;
        g_eclassToken[1] = 0;
        return text + 1;
    }

    do                                               // word (incl. lone '/')
    {
        ++text;
        g_eclassToken[len++] = c;
        c = *text;
    }
    while ( !IsTokenSpecial( c ) && c > 32 );
    g_eclassToken[len] = 0;
    return text;
}

// ─────────────────────────────────────────────────────────────────────────────
// Eclass_InitFromText  (0x481150)
// Parse one /*QUAKED <name> (r g b) [size] <flags...>\n<comments>*/ block into a
// fresh eclass_t. Uses Eclass_GetToken (above), sscanf for the color/bbox, the six
// Init_CStr attribute extractors (defaultmdl=/model=/previewmdl1-4=, keys recovered
// from the binary's ecx args), Eclass_ParseDefaults, and the classtype switch.
// 'text' points at the "/*QUAKED" marker (binary's __thiscall 'this').
// ─────────────────────────────────────────────────────────────────────────────
eclass_t *Eclass_InitFromText( void *textPtr )
{
    char *text = (char *)textPtr;

    eclass_t *ec = (eclass_t *)operator new( 0x184u );
    memset( ec, 0, 0x184u );

    // Entity name = first token after "/*QUAKED " (text + 9).
    char *v3 = Eclass_GetToken( text + 9 );
    ec->name = (char *)operator new( strlen( g_eclassToken ) + 1 );
    strcpy( ec->name, g_eclassToken );

    ec->color[0] = ec->color[1] = ec->color[2] = 1.0f;
    ec->unk = 1.0f;
    g_eclassCurrentName = (int)ec->name;

    // Color block " (r g b)".
    if ( sscanf( v3, " (%g %g %g)", &ec->color[0], &ec->color[1], &ec->color[2] ) != 3 )
        return ec;

    SetMaterial( "$opaque", (patchMesh_material *)ec->material );

    // Advance to the ')' that closes the color block.
    char *q = v3;
    if ( *q != ')' )
    {
        char ch = *q;
        while ( ch ) { ch = *++q; if ( ch == ')' ) break; }
        if ( ch != ')' )
            return ec;
    }

    char *v11 = Eclass_GetToken( q + 1 );

    // Optional fixed-size bbox: next token is '('.
    if ( g_eclassToken[0] == '(' )
    {
        *(int *)&ec->fixedsize = 1;
        if ( sscanf( v11, "%f %f %f) (%f %f %f)",
                     &ec->mins[0], &ec->mins[1], &ec->mins[2],
                     &ec->maxs[0], &ec->maxs[1], &ec->maxs[2] ) != 6 )
            return ec;   // malformed size block → name+color+fixedsize only

        // Skip past the two ')' that close the mins/maxs blocks.
        // Disasm 0x481280: the binary returns the whole eclass
        // (goto loc_48159E) if it hits a NUL before finding both ')' — it does NOT
        // fall through to the flag/comment parse.  The prior `while(*v11)` exited on
        // NUL and proceeded to LABEL_14 with v11 at the terminator.  In practice this
        // branch is unreachable: the loop only runs after the mins/maxs sscanf above
        // returned 6, which requires both ')' to be present — so a NUL can't precede
        // them.  Transcribed to mirror the binary's structure exactly anyway (an audit
        // should match the disasm even on dead edges).
        int closed = 0;
        while ( 1 )
        {
            if ( *v11 == ')' )
            {
                ++v11;
                if ( ++closed >= 2 )
                    break;
            }
            else if ( !*v11 )
            {
                return ec;        // loc_48159E: bail with name+color+fixedsize only
            }
            else
            {
                ++v11;
            }
        }
    }

    // ── Flags + comments (LABEL_14) ───────────────────────────────────────────
    // 1. Copy the remainder of the QUAKED line (the flag-name list) until '\n'.
    char descLine[260];
    {
        char *o = descLine;
        char ch = *v11;
        while ( *v11 )
        {
            if ( ch == '\n' ) break;
            ++v11;
            *o++ = ch;
            ch = *v11;
        }
        *o = 0;
    }
    char *afterLine = v11 + 1;   // first char past the '\n'

    // 2. Tokenize the flag line into up to 8 flag names (ec+0x40 + 32*k).
    {
        char *tp = descLine;
        for ( int k = 0; k < 8; ++k )
        {
            tp = Eclass_GetToken( tp );
            if ( !tp )
                break;
            strcpy( (char *)ec + 64 + 32 * k, g_eclassToken );
        }
    }

    // 3. Comments = text from afterLine up to "*/".
    {
        char *j = afterLine;
        while ( *j )
        {
            if ( *j == '*' && j[1] == '/' )
                break;
            ++j;
        }
        size_t clen = (size_t)( j - afterLine );
        ec->comments = (char *)operator new( clen + 1 );
        memcpy( ec->comments, afterLine, clen );
        ec->comments[clen] = 0;
    }

    // 4. Extract the model/preview attributes from the comments.
    Init_CStr( ec, (void **)&ec->default_model_name, "defaultmdl=" );
    Init_CStr( ec, (void **)&ec->modelpath,          "model=" );
    Init_CStr( ec, (void **)&ec->xx3,                "previewmdl1=" );
    Init_CStr( ec, (void **)&ec->xx4,                "previewmdl2=" );
    Init_CStr( ec, (void **)&ec->xx5,                "previewmdl3=" );
    Init_CStr( ec, (void **)&ec->xx6,                "previewmdl4=" );

    // 5. Register any "default: key value" attributes.
    Eclass_ParseDefaults( ec );

    // 6. classtype flags by name.
    ec->classtype = 0;
    if ( _stricmp( ec->name, "worldspawn" ) )
        ec->classtype |= 2;
    if ( !_stricmp( ec->name, "light" ) )            { ec->classtype |= 1;     return ec; }
    if ( !_stricmp( ec->name, "reflection_probe" ) ) { ec->classtype |= 0x100; return ec; }
    if ( !_stricmp( ec->name, "path" ) )             { ec->classtype |= 4;     return ec; }
    if ( !_stricmp( ec->name, "misc_model" )    || !_stricmp( ec->name, "script_model" ) ||
         !_stricmp( ec->name, "script_vehicle" )|| !_stricmp( ec->name, "dyn_model" ) )
                                                     { ec->classtype |= 8;     return ec; }
    if ( !_stricmp( ec->name, "misc_prefab" ) )      { ec->classtype |= 0x10;  return ec; }
    if ( !_stricmp( ec->name, "trigger_radius" ) )   { ec->classtype |= 0x40;  return ec; }
    if ( !_stricmp( ec->name, "trigger_disk" ) )     { ec->classtype |= 0x80;  return ec; }
    if ( !strncmp( ec->name, "node_", 5 ) )
        ec->classtype |= 0x20;

    return ec;
}


// ─────────────────────────────────────────────────────────────────────────────
// Eclass_InsertAlphabetized  (0x4817E0)
// Insert eclass node a2 into the alphabetized singly-linked list *a1
// (ordered by name, stored at field [1]).
// ─────────────────────────────────────────────────────────────────────────────
void Eclass_InsertAlphabetized(const char ***a1, const char **a2)
{
    const char **v2 = *a1;
    if ( !*a1 )
        goto LABEL_4;

    if ( _stricmp(a2[1], v2[1]) < 0 )
    {
        *a2 = (const char *)v2;
LABEL_4:
        *a1 = a2;
        return;
    }

    for ( ; *v2; v2 = (const char **)*v2 )
    {
        if ( _stricmp(a2[1], *((const char **)*v2 + 1)) < 0 )
            break;
    }
    *a2 = *v2;
    *v2 = (const char *)a2;
}


// ─────────────────────────────────────────────────────────────────────────────
// sub_4815C0  ->  Eclass_LoadModel  (0x4815C0)
// Load the model(s)/prefab(s) named in ec->modelpath (semicolon-separated) into a
// linked list of 0x98-byte sub-model nodes headed by ec->xx1 (+0x160). Each name is
// loaded by Prefab_Load (classtype prefab bit 0x10) or Model_load, into the current
// node; a fresh empty node is appended after each. modelpath is freed/cleared on exit.
// The binary uses an MFC CStringList (sub_5AC540 ctor / sub_5ACA05 AddTail / GetAt /
// sub_5AC676 dtor); with MFC enabled this is a real CStringList walked head→tail.
// ─────────────────────────────────────────────────────────────────────────────
void Eclass_LoadModel(int a1)
{
    eclass_t *ec = (eclass_t *)a1;
    CStringList list;   // IDA sub_5AC540(&v9, 10) — 10 is only an allocation hint

    // ec->xx1 (+0x160) heads the sub-model node list; create the first node if absent.
    if ( !ec->xx1 )
    {
        void *node = operator new( 0x98u );
        memset( node, 0, 0x98u );
        ec->xx1 = (int)node;
    }

    // Split modelpath on ';' into the list (operate on a copy — strtok is destructive).
    char *copy = (char *)AllocMaterialString( (const char *)ec->modelpath );
    for ( char *tok = strtok( copy, ";" ); tok; tok = strtok( nullptr, ";" ) )
        list.AddTail( tok );

    int      v4    = ec->xx1;          // current (tail) node
    int      j     = v4;               // node handed to the loader
    POSITION pos   = list.GetHeadPosition();
    int      count = (int)list.GetCount();

    for ( int idx = 0; idx < count; ++idx )
    {
        CString name = list.GetNext( pos );   // forward walk == IDA's GetAt(idx)
        if ( ec->classtype & 0x10 )
            Prefab_Load( (LPCSTR)name, &j );
        else
            Model_load( (char *)(LPCSTR)name, (int)&j );

        int newNode = (int)operator new( 0x98u );
        *(int *)v4 = newNode;          // link: *v4 = newNode
        v4 = newNode;
        j  = newNode;
        memset( (void *)newNode, 0, 0x98u );
    }

    j__free_0( copy );
    j__free_0( (void *)ec->modelpath );
    ec->modelpath = 0;
    // CStringList destructor (sub_5AC676) runs automatically here.
}


// ─────────────────────────────────────────────────────────────────────────────
// Eclass_hasModel  (0x481740)
// Returns true if the eclass has a loadable XModel (handle valid, not error
// placeholder) or has sub-models (xx1+12 non-zero).
// ─────────────────────────────────────────────────────────────────────────────
bool Eclass_hasModel(eclass_t *e)
{
    iassert(e);

    // If modelpath is set, attempt to load it now (converts path → handle).
    if ( e->modelpath )
    {
        Eclass_LoadModel((int)e);
        if ( e->modelpath )
        {
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
                   522, 1, "%s", "e->modelpath == NULL");
        }
    }

    int xx1 = e->xx1;
    if ( !xx1 )
        return false;

    // xx1 is a models_t* for this eclass.
    // Field +4 = primary handle (XModel*), +12 = sub-model list head.
    // CoD3 != CoD4: the IDB reads the XModel "bad" flag at +193 (CoD4
    // XModel layout), but kisak's CoD3 XModel has `bad` at +205 — reading +193 hits a
    // lodInfo byte (garbage, e.g. 208), so a freshly-loaded misc_model xmodel was wrongly
    // judged "bad" and never instanced.  Use kisak's XModelBad() (reads the right field).
    extern bool XModelBad( const XModel *model );
    int v3 = *(int *)(xx1 + 4);
    return (v3 && !XModelBad( (const XModel *)v3 )) || (*(int *)(xx1 + 12) != 0);
}


// ─────────────────────────────────────────────────────────────────────────────
// sub_482260  →  Eclass_ModelLoaded  (0x482260)
// Return true if a model eclass with the given name exists in g_models and
// has a valid model.
// ─────────────────────────────────────────────────────────────────────────────
char Eclass_ModelLoaded(const char *modelName)
{
    iassert(modelName);
    iassert(modelName[0]);

    eclass_t *e = (eclass_t *)g_models;
    if ( !g_models )
        return 0;

    while ( _stricmp(modelName, e->name) )
    {
        e = e->next;
        if ( !e )
            return 0;
    }

    iassert(Eclass_hasModel( e ));
    return 1;
}


// ─────────────────────────────────────────────────────────────────────────────
// Eclass_ForName  (0x482190)
// Look up an eclass by name in g_eclass.  If not found, create a placeholder
// using Eclass_InitFromText with a synthetic QUAKED block.
//   has_brushes = 0: entity has fixed bounding box (-8 -8 -8) (8 8 8)
//   has_brushes = 1: entity is brush-based (no size block, just '?')
// ─────────────────────────────────────────────────────────────────────────────
eclass_t *Eclass_ForName(int has_brushes, const char *name)
{
    if ( !name || !*name )
        return eclass_bad;

    eclass_t *v3 = g_eclass;
    if ( g_eclass )
    {
        while ( _stricmp(name, v3->name) )
        {
            v3 = v3->next;
            if ( !v3 )
                goto LABEL_6;
        }
        return v3;
    }

LABEL_6:
    {
        char v6[1028];
        if ( has_brushes )
        {
            sprintf(v6,
                "/*QUAKED %s (0 0.5 0) ?\nNot found in source.\n",
                name);
        }
        else
        {
            sprintf(v6,
                "/*QUAKED %s (0 0.5 0) (-8 -8 -8) (8 8 8)\nNot found in source.\n",
                name);
        }
        const char **v5 = (const char **)Eclass_InitFromText(v6);
        Eclass_InsertAlphabetized((const char ***)&g_eclass, v5);
        return (eclass_t *)v5;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Eclass_01  (0x482300)
// Look up or allocate a per-map models_t entry for the entity at offset 96 of
// the structure pointed to by a2.
//
// IDA signature: models_t *__usercall Eclass_01@<eax>(const char *a1@<eax>, int a2)
//   a1 = model name
//   a2 = entity context pointer; *(a2+96) is the eclass_t*, *(a2+100) is out-ptr.
//
// The model-type flag (bit 4 = 0x10) is at eclass_t byte offset 384 (classtype).
// ─────────────────────────────────────────────────────────────────────────────
models_t *Eclass_01(const char *a1, int a2)
{
    if ( !a1 || !strlen(a1) )
        return nullptr;

    int e = g_models;
    if ( !g_models )
        goto LABEL_7;

    // Search g_models for a node whose name matches a1 AND whose model-type
    // flag (bit 4) matches the entity's eclass flag.
    while ( _stricmp(a1, *(const char **)(e + 4)) ||
            ((*(unsigned char *)(e + 384) ^ *(unsigned char *)(*(int *)(a2 + 96) + 384)) & 0x10) != 0 )
    {
        e = *(int *)e;
        if ( !e )
            goto LABEL_7;
    }

    // Found existing node.
    if ( !Eclass_hasModel((eclass_t *)e) )
    {
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\eclass.cpp",
               860, 0, "%s", "Eclass_hasModel( e )");
    }
    *(int *)(a2 + 100) = e;
    return (models_t *)e;

LABEL_7:
    {
        DisableRendering_pp();

        models_t *models = (models_t *)operator new(0x184u);
        memset(models, 0, sizeof(models_t));

        unsigned int v5 = (unsigned int)strlen(a1);
        void *v11 = operator new(v5 + 1);
        memcpy_0(v11, a1, v5 + 1);
        models->handle = (int)v11;

        unsigned int v6 = (unsigned int)strlen(a1);
        void *v12 = operator new(v6 + 1);
        memcpy_0(v12, a1, v6 + 1);
        models->x94 = (int)v12;

        // Copy the model-type flag bit from the entity's eclass.
        int v7 = *(int *)(*(int *)(a2 + 96) + 384);
        *(float *)&models->x11 = 0.85000002f;
        *(float *)&models->x9  = 0.85000002f;
        models->x96 |= v7 & 0x10;

        // Seed fields from the eclass found (or created) by Eclass_ForName.
        int *v8 = (int *)Eclass_ForName(0, a1);
        models->entities_next__substruct = v8[3];
        models->x4 = v8[4];
        models->x5 = v8[5];
        models->x6 = v8[6];
        models->x7 = v8[7];
        models->x8 = v8[8];

        const char *v9 = (const char *)v8[89];
        if ( v9 )
        {
            models->x89 = (int)AllocMaterialString(v9);
        }

        if ( Eclass_hasModel((eclass_t *)models) )
        {
            Eclass_InsertAlphabetized((const char ***)&g_models, (const char **)models);
            *(int *)(a2 + 100) = (int)models;
            DisableRendering_mm();
            return models;
        }

        Eclass_CheckEntities(models);
        DisableRendering_mm();
        return nullptr;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Init_ScanFiles  (0x481840)
// Read a single .def file and parse all /*QUAKED blocks into g_eclass.
//
// IDA signature: void __cdecl Init_ScanFiles(int a1)
//   a1 = 0: use LoadFile (raw filesystem)
//   a1 = 1: use FS_ReadFile (VFS)
//
// The original binary receives the file path in ecx (a __usercall-style register
// argument set by the sprintf caller in Eclass_InitForSourceDirectory).  In this
// port the caller writes the path into g_scanFilePath before calling.
// ─────────────────────────────────────────────────────────────────────────────
void Init_ScanFiles(int a1)
{
    const char *filePath = g_scanFilePath;

    // Normalise backslashes → forward slashes for display/strstr.
    char v22[1024];
    {
        const char *v2 = filePath;
        char v3 = *v2;
        char *i = v22;
        for ( ; v3; ++i )
        {
            *i = (v3 == '\\') ? '/' : v3;
            v3 = i[v2 - v22 + 1];
        }
        *i = 0;
    }

    if ( strstr(v22, "/.#") )
    {
        Sys_Printf("IngoringFile: %s\n", v22);
        return;
    }

    Sys_Printf("ScanFile: %s\n", v22);

    void  *buffer   = nullptr;
    int    fileSize;
    if ( a1 )
    {
        fileSize = (int)FS_ReadFile(filePath, &buffer);
    }
    else
    {
        fileSize = (int)LoadFile(filePath, &buffer);
    }

    int v6 = 0;
    g_eclassParsedAny = 0;

    for ( int j = 0; v6 < fileSize; j = ++v6 )
    {
        if ( strncmp((const char *)buffer + v6, "/*QUAKED", 8u) )
            continue;

        char *v15 = (char *)buffer + v6;

        if ( g_bBuildList )
        {
            // Accumulate the raw QUAKED block text into lpBuffer.
            void *Src = nullptr;
            cstr(&Src, zero);
            int v23 = 0;
            while ( 1 )
            {
                unsigned char *v7 = (unsigned char *)Src;
                int  v8  = *((int *)Src - 3);
                int  v9  = *((int *)Src - 2);
                char v20 = *((char *)buffer + v6);
                int  v10 = v8 + 1;
                if ( ((1 - *((int *)Src - 1)) | (v9 - v10)) < 0 )
                {
                    __strcpy(&Src, v10);
                    v7 = (unsigned char *)Src;
                }
                v7[v8] = (unsigned char)v20;
                if ( v10 < 0 || v10 > *((int *)Src - 2) )
                    OleException(-2147024809);
                *((int *)Src - 3) = v10;
                unsigned char *v11 = (unsigned char *)buffer;
                *((unsigned char *)Src + v10) = 0;
                if ( v11[v6] == '/' && v11[v6 - 1] == '*' )
                    break;
                ++v6;
            }
            str_append(&Src, (void *)"\r\n\r\n\r\n", 6u);
            str_append(&lpBuffer, Src, *((int *)Src - 3));
            v23 = -1;
            char *v12 = (char *)Src - 16;
            if ( _InterlockedDecrement((volatile long *)Src - 1) <= 0 )
            {
                (*(void (__stdcall **)(char *))(**(int **)v12 + 4))(v12);
            }
        }

        const char **v13 = (const char **)Eclass_InitFromText(v15);
        if ( v13 )
        {
            Eclass_InsertAlphabetized((const char ***)&g_eclass, v13);
        }
        else
        {
            printf("Error parsing: %s in %s\n",
                   (const char *)g_eclassCurrentName, filePath);
        }

        g_lastParsedEclass = (int)v13;
        g_eclassParsedAny  = 1;

        if ( g_eclassStopOnFirst )
            break;

        v6 = j;
    }

    if ( a1 )
    {
        if ( !fs_searchpaths )
        {
            FatalError(0, "Filesystem call made without initialization\n");
        }
        void *v14 = buffer;
        if ( !buffer )
        {
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_files.cpp",
                   2840, 0, "%s", "buffer");
        }
        --fs_loadStack;
        Hunk_FreeTempMemory((char *)v14);
    }
    else
    {
        free(buffer);
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Eclass_InitForSourceDirectory  (0x481B50)
// Scan a glob pattern (e.g. "scripts/*.def") for /*QUAKED entity definitions.
// Handles up to two additional source directories derived from the path.
// At the end, creates the UNKNOWN_CLASS fallback eclass → eclass_bad.
//
// IDA signature: eclass_t *__thiscall Eclass_InitForSourceDirectory(char *this)
//   'this' is the source directory glob path string.
// ─────────────────────────────────────────────────────────────────────────────
eclass_t *Eclass_InitForSourceDirectory(char *path)
{
    char v54[1024];   // normalised (forward-slash) copy of path
    char v55[1024];   // base directory (path with filename stripped)
    char v53[1024];   // full path for each enumerated file
    char v51;         // first char of subdirectory basename
    char v52[1023];   // rest of subdirectory basename
    char FileName[1024]; // additional source directory path
    _finddata64i32_t v49; // find-file data block (_findfirst64i32 output)
    char *v50 = v49.name; // cFileName pointer (name field in _finddata64i32_t)

    // Normalise backslashes → forward slashes.
    {
        char v2 = *path;
        const char *src = path;
        char *dst = v54;
        for ( ; v2; ++dst )
        {
            *dst = (v2 == '\\') ? '/' : v2;
            v2 = dst[src - v54 + 1];
        }
        *dst = 0;
    }

    Sys_Printf("Eclass_InitForSourceDirectory: %s\n", v54);
    strcpy(v55, path);

    // Strip the filename to get the base dir: null-terminate at the last path
    // separator so v55 = the directory that sprintf("%s\\%s", v55, foundName)
    // rebuilds each enumerated file's full path from.
    //
    // STACK-ADJACENCY ARTIFACT: the binary walks `j = &v54[strlen(v55)+1023]`
    // — which equals &v55[strlen(v55)-1] (the LAST char of the raw path v55) ONLY
    // because the binary's stack frame places v55 immediately after v54[1024].
    // The port's v54/v55 are separate locals with no such adjacency, so that
    // literal pointer arithmetic lands in garbage and corrupts v55 (→ every
    // sprintf path is wrong → Init_ScanFiles reads nothing → cod4.def registered
    // ZERO eclasses).  Express the binary's actual INTENT directly on v55.
    {
        char *j = v55 + (int)strlen(v55) - 1;      // == the binary's &v54[strlen+1023]
        while ( *j != '\\' )
        {
            if ( *j == '/' )   break;
            if ( j == v55 )    break;
            --j;
        }
        *j = 0;
    }

    Eclass_FreeAll();
    g_eclass = nullptr;

    if ( g_bBuildList )
    {
        str_set((void **)&lpBuffer, (void *)"// THIS FILE IS AUTOGENERATED, DO NOT MODIFY \n\n", 0x2Fu);
    }

    // ── First pass: scan the primary directory. ──────────────────────────────
    intptr_t hFind = _findfirst64i32(path, &v49);
    if ( hFind != -1 )
    {
        do
        {
            sprintf(v53, "%s\\%s", v55, v50);
            strcpy(g_scanFilePath, v53);
            Init_ScanFiles(0);
        }
        while ( _findnext64i32(hFind, &v49) != -1 );
        _findclose(hFind);
    }

    // ── Additional source directories (only when path does not end in ".def") ─
    if ( !strstr(path, ".def") )
    {
        // Copy path into FileName.
        strcpy(FileName, path);

        int v9 = (int)strlen(FileName) - 1;
        if ( v9 > 0 )
        {
            // Find last separator in FileName.
            while ( 1 )
            {
                char v10 = FileName[v9];
                if ( v10 == '/' || v10 == '\\' )
                    break;
                if ( --v9 <= 0 )
                    goto LABEL_72;
            }

            // Copy subdir name (after separator) into v51/v52.
            char *v11 = &FileName[v9];
            char *v12 = &FileName[v9];
            {
                char v13;
                do { v13 = *v12; v12[&v51 - v11] = *v12; ++v12; } while ( v13 );
            }
            *v11 = 0;

            // Append the "_mp" marker + subdirname to v55.
            // IDA: `mov ecx, ds:off_6E9644; mov [edi], ecx` — a 4-byte store of the inline
            // string at 0x6E9644, which is "_mp\0" (== 0x00706D5F).  Hex-rays mistyped those
            // bytes as a pointer (&unk_706D5F) because 0x706D5F looks like a valid address;
            // they are actually the ASCII marker "_mp" (magic-address artifact).  The prior
            // port read it as "m_" — wrong byte interpretation.
            strcat(v55, "_mp");
            {
                unsigned int len16 = (unsigned int)strlen(&v51) + 1;
                char *ptr = v55 + strlen(v55);
                qmemcpy(ptr, &v51, len16);
            }

            // Normalise FileName for display.
            {
                char v19 = FileName[0];
                char *k = v54;
                for ( ; v19; ++k )
                {
                    *k = (v19 == '\\') ? '/' : v19;
                    v19 = k[FileName - v54 + 1];
                }
                *k = 0;
            }
            Sys_Printf("AdditionalSourceDirectory: %s\n", v54);

            // Write FileName into v55.
            {
                int idx = 0; char c;
                do { c = FileName[idx]; v55[idx++] = c; } while ( c );
            }

            // Recalculate base dir from updated v55. (Same stack-adjacency artifact
            // as the primary strip above — operate on v55 directly.)
            {
                char *m = v55 + (int)strlen(v55) - 1;
                while ( *m != '\\' )
                {
                    if ( *m == '/' )   break;
                    if ( m == v55 )    break;
                    --m;
                }
                *m = 0;
            }

            intptr_t hFind2 = _findfirst64i32(FileName, &v49);
            if ( hFind2 != -1 )
            {
                do
                {
                    sprintf(v53, "%s\\%s", v55, v50);
                    strcpy(g_scanFilePath, v53);
                    Init_ScanFiles(0);
                }
                while ( _findnext64i32(hFind2, &v49) != -1 );
                _findclose(hFind2);
            }

            // Recopy original path into FileName for second-level scan.
            strcpy(FileName, path);

            int v27 = (int)strlen(FileName) - 1;
            int n   = 0;
            for ( ; v27 > 0; --v27 )
            {
                char v29 = FileName[v27];
                if ( v29 == '/' || v29 == '\\' )
                {
                    if ( n )
                    {
                        // Second separator: extract sub-path.
                        char *v30 = &FileName[v27];
                        char *v31 = &FileName[v27];
                        {
                            char v32;
                            do { v32 = *v31; v31[&v51 - v30] = *v31; ++v31; } while ( v32 );
                        }
                        *v30 = 0;

                        // Append "/b" + subdir.
                        {
                            char *ptr = &v55[1023];
                            while ( *++ptr ) { }
                            strcpy(ptr, "/b");
                        }
                        {
                            unsigned int len35 = (unsigned int)strlen(v52) + 1;
                            char *ptr = &v55[1023];
                            while ( *++ptr ) { }
                            qmemcpy(ptr, v52, len35);
                        }

                        // Normalise for display.
                        {
                            char v38 = FileName[0];
                            char *ii = v54;
                            for ( ; v38; ++ii )
                            {
                                *ii = (v38 == '\\') ? '/' : v38;
                                v38 = ii[FileName - v54 + 1];
                            }
                            *ii = 0;
                        }
                        Sys_Printf("AdditionalSourceDirectory: %s\n", v54);

                        {
                            int idx = 0; char c;
                            do { c = FileName[idx]; v55[idx++] = c; } while ( c );
                        }

                        {
                            char *jj = v55 + (int)strlen(v55) - 1;   // stack-adjacency artifact; see primary strip
                            while ( *jj != '\\' )
                            {
                                if ( *jj == '/' )   break;
                                if ( jj == v55 )    break;
                                --jj;
                            }
                            *jj = 0;
                        }

                        intptr_t hFind3 = _findfirst64i32(FileName, &v49);
                        if ( hFind3 != -1 )
                        {
                            do
                            {
                                sprintf(v53, "%s\\%s", v55, v50);
                                strcpy(g_scanFilePath, v53);
                                Init_ScanFiles(0);
                            }
                            while ( _findnext64i32(hFind3, &v49) != -1 );
                            _findclose(hFind3);
                        }
                        break;
                    }
                    n = 1;
                }
            }
        }
    }

LABEL_72:
    if ( g_bBuildList )
    {
        // Write accumulated QUAKED blocks to "newdefs.def".
        // IDB: CFile::Open (instance method), Write, Close, ~CFile.
        CFile v47;
        if ( v47.Open("newdefs.def", 0x1001u) )
        {
            LPCVOID v44 = lpBuffer;
            DWORD   v45 = *((DWORD *)lpBuffer - 3);
            if ( ((1 - *((int *)lpBuffer - 1)) | *((int *)lpBuffer - 2)) < 0 )
            {
                __strcpy(&lpBuffer, 0);
                v44 = lpBuffer;
            }
            v47.Write(v44, v45);
            v47.Close();
        }
        // v47 destructor called automatically at end of scope.
    }

    eclass_t *result = Eclass_InitFromText((void *)"/*QUAKED UNKNOWN_CLASS (0 0.5 0) ?");
    eclass_bad = result;
    return result;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Weapon / AI def-folder eclass scanners.  Load_Defs / ScanWeapAiFiles synthesize weapon
//  (and AI-type) entity classes from the loose def files so they appear in the entity RMB
//  menu; Eclass_InitForSourceDirectory only scans .def files and would miss them.
// ═════════════════════════════════════════════════════════════════════════════
extern const char  *Info_ValueForKey( const char *s, const char *key );        // q_shared 0x4B98D0
extern int          Com_sprintf( char *dest, uint32_t size, const char *fmt, ... );  // q_shared 0x4B9780

// Init_ScanWeapons (0x48B7B0) — read each weapon def file in `folder`, extract its
// displayName/worldModel, and synthesize a `weapon_<name>` /*QUAKED*/ eclass block.
void Init_ScanWeapons( const char *folder )
{
    int  weaponPrefixLen = (int)strlen( "WEAPONFILE" );
    if ( !fs_searchpaths )
        FatalError( 0, "Filesystem call made without initialization\n" );

    int          count = 0;
    const char **list  = folder ? FS_ListFiles( folder, "", FS_LIST_ALL, &count ) : nullptr;

    for ( int i = 0; i < count; ++i )
    {
        const char *fname = list[i];
        char qpath[1024];
        sprintf( qpath, "%s\\%s", folder, fname );
        Sys_Printf( "ScanWeapon: %s\n", qpath );

        char *buffer = nullptr;
        if ( (int)FS_ReadFile( qpath, (void **)&buffer ) > weaponPrefixLen )
        {
            if ( !strncmp( buffer, "WEAPONFILE", weaponPrefixLen ) )
            {
                char *info = &buffer[weaponPrefixLen];
                // the info string must be well-formed (no stray quotes / semicolons).
                if ( !strchr( info, '"' ) && !strchr( info, ';' ) )
                {
                    char className[1024];
                    Com_sprintf( className, 0x400, "weapon_%s", fname );
                    char displayName[1024];
                    char worldModel[68];
                    strcpy( displayName, Info_ValueForKey( info, "displayName" ) );
                    strcpy( worldModel,  Info_ValueForKey( info, "worldModel" ) );

                    char quaked[1024];
                    sprintf( quaked,
                             "/*QUAKED %s (.3 .3 1) (-16 -16 -16) (16 16 16) SUSPENDED SPIN - RESPAWN\n"
                             "defaultmdl=\"%s\"\n%s\nInternal(script) name:%s\n"
                             "\"count\" - Specifies a specific amount of ammo to include with the weapon. "
                             "Amount is capped to ammo max.",
                             className, worldModel, displayName, qpath );
                    const char **ecls = (const char **)Eclass_InitFromText( quaked );
                    if ( ecls )
                        Eclass_InsertAlphabetized( (const char ***)&g_eclass, ecls );
                }
            }
        }
        if ( !fs_searchpaths )
            FatalError( 0, "Filesystem call made without initialization\n" );
        if ( !buffer )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_files.cpp",
                    2840, 0, "%s", "buffer" );
        --fs_loadStack;
        Hunk_FreeTempMemory( buffer );
    }
    if ( list )
        FS_FreeFileList( list );
}

// ScanWeapAiFiles (0x48BA40) — scan the single- and multi-player weapon folders.
void ScanWeapAiFiles()
{
    // (the binary walks d_project_entity->epairs for "basepath" — read but unused here;
    // the two weapon folders are fixed relative paths under the searchpaths.)
    Sys_Printf( "Scanning single player weapons: %s/*\n", "weapons/sp" );
    Init_ScanWeapons( "weapons/sp" );
    Sys_Printf( "Scanning multiplayer weapons: %s/*\n", "weapons/mp" );
    Init_ScanWeapons( "weapons/mp" );
}

// ── Load_Defs (0x48B6C0) — scan an AI-types def folder for /*QUAKED*/ eclass blocks ─────
// The binary lists every *.gsc file under `folder` in the searchpaths and, for each,
// sprintf's "<folder>\<file>" into the scan path and calls Init_ScanFiles(1) (FS_ReadFile
// path).  Wired from QE_LoadProject with the AI-types + gametypes folders.  In the port,
// Init_ScanFiles reads the path from g_scanFilePath (the ecx-arg bridge), so we seed that
// before each call.  Uses FS_ListFiles(folder,"gsc",...) — kisak's equivalent of the binary's
// older FS_ListFilteredFiles(folder,"gsc") (list files with extension under a searchpath dir).
void Load_Defs( const char *folder )
{
    if ( !fs_searchpaths )
        FatalError( 0, "Filesystem call made without initialization\n" );
    if ( !folder )
        return;

    int          count = 0;
    const char **list  = FS_ListFiles( folder, "gsc", FS_LIST_ALL, &count );
    Sys_Printf( "Scanning AI types folder: %s/*gsc\n", folder );

    for ( int i = 0; i < count; ++i )
    {
        sprintf( g_scanFilePath, "%s\\%s", folder, list[i] );
        Init_ScanFiles( 1 );
    }
    if ( list )
        FS_FreeFileList( list );
}
